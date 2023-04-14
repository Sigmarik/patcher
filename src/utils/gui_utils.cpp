#include "gui_utils.h"

#include <stdlib.h>
#include <stdarg.h>

#include <x86intrin.h>

#include "lib/util/dbg/logger.h"
#include "lib/util/dbg/debug.h"

void print_label() {
    printf("GUI for binary patcher by Ilya Kudryashov.\n");
    printf("GUI for binary patcher with test animation.\n");
    printf("Build from\n%s %s\n", __DATE__, __TIME__);
    log_printf(ABSOLUTE_IMPORTANCE, "build info", "Build from %s %s.\n", __DATE__, __TIME__);
}

static sf::Vector3f to_sf_vec3(glm::vec3 v) {return sf::Glsl::Vec3(v.x, v.y, v.z);}

void set_camera_uniforms(const Camera camera, sf::Shader* shader) {
    shader->setUniform("cam_pos",     to_sf_vec3(camera.position));
    shader->setUniform("cam_forward", to_sf_vec3(glm::normalize(camera.forward)));
    shader->setUniform("cam_up",      to_sf_vec3(glm::normalize(camera.up)));
    shader->setUniform("cam_right",   to_sf_vec3(glm::normalize(cross(camera.forward, camera.up))));
    shader->setUniform("cam_fov",     camera.fov);
    shader->setUniform("cam_ar",      camera.aspect_ratio);
}

void init_scene(RenderScene* scene) {
    scene->camera = (Camera) {.position = glm::vec3(0.0f, 0.0f, 0.0f),
                              .forward  = glm::vec3(0.0f, 0.0f, -1.0f),
                              .up       = glm::vec3(0.0f, 1.0f, 0.0f),
                              .fov = 90};
    scene->time = sf::Time::Zero;
    scene->last_update_time = sf::Time::Zero;

    _LOG_FAIL_CHECK_(scene->background.loadFromFile(BACKGROUND_NAME), "error", ERROR_REPORTS, return, &errno, ENOENT);
    _LOG_FAIL_CHECK_(scene->foreground.loadFromFile(FOREGROUND_NAME), "error", ERROR_REPORTS, return, &errno, ENOENT);

    scene->display_texture.create(SCREEN_WIDTH, SCREEN_HEIGHT);
    scene->display_sprite = sf::Sprite(scene->display_texture);

    scene->camera.aspect_ratio = (float) SCREEN_WIDTH / (float) SCREEN_HEIGHT;
}

void scene_dtor(RenderScene* scene) {
    scene->foreground.~Image();
    scene->background.~Image();
    scene->display_sprite.~Sprite();
    scene->display_texture.~Texture();
    scene->time = sf::Time::Zero;
    scene->last_update_time = sf::Time::Zero;
    scene->camera = (Camera) {};
}

void update_scene(RenderScene* scene, sf::RenderWindow* window, sf::Time dt) {
    sf::Vector2i mouse_pos = sf::Mouse::getPosition(*window);
    glm::vec2 rel_mouse_pos = glm::vec2(mouse_pos.x, mouse_pos.y) * 2.0f / glm::vec2(SCREEN_WIDTH, SCREEN_HEIGHT) - 
                              glm::vec2(1.0f, 1.0f);
    rel_mouse_pos.y *= -1.0f;

    #ifndef EXPERIMENT
    scene->foreground_position = sf::Vector2i(mouse_pos.x - (int) scene->foreground.getSize().x / 2,
                                              mouse_pos.y - (int) scene->foreground.getSize().y / 2);
    #else
    scene->foreground_position = sf::Vector2i(0, 0);
    #endif

    scene->time += dt;
    scene->last_update_time = scene->time;
}

// It uses the most simple and basic linear algorithm of alpha blending.
static inline __m256i mix_colors(__m256i back, __m256i front) {
    const char O = 0, I = (char) 255, Z = (char) 0x80;
    const __m256i LEFT_SPLIT = _mm256_set_epi8(
    Z, 29, Z, 28,  Z, 25, Z, 24,  Z, 21, Z, 20,  Z, 17, Z, 16,  Z, 13, Z, 12,  Z, 9, Z, 8,    Z, 5, Z, 4,  Z, 1, Z, 0
    );
    const __m256i RIGHT_SPLIT = _mm256_set_epi8(
    Z, 31, Z, 30,  Z, 27, Z, 26,  Z, 23, Z, 22,  Z, 19, Z, 18,  Z, 15, Z, 14,  Z, 11, Z, 10,  Z, 7, Z, 6,  Z, 3, Z, 2
    );
    const __m256i ALPHA_EXTRACTOR = _mm256_set_epi8(
    Z, 31, Z, 31,  Z, 27, Z, 27,  Z, 23, Z, 23,  Z, 19, Z, 19,  Z, 15, Z, 15,  Z, 11, Z, 11,  Z, 7, Z, 7,  Z, 3, Z, 3
    );
    const __m256i WHITE = _mm256_set_epi8(
    O, I, O, I,    O, I, O, I,    O, I, O, I,    O, I, O, I,    O, I, O, I,    O, I, O, I,    O, I, O, I,  O, I, O, I
    );

    const __m256i ASSEMBLE_LEFT = _mm256_set_epi8(
    Z, Z, 31, 29,  Z, Z, 27, 25,  Z, Z, 23, 21,  Z, Z, 19, 17,  Z, Z, 15, 13,  Z, Z, 11, 9,  Z, Z, 7, 5,  Z, Z, 3, 1
    );
    const __m256i ASSEMBLE_RIGHT = _mm256_set_epi8(
    31, 29, Z, Z,  27, 25, Z, Z,  23, 21, Z, Z,  19, 17, Z, Z,  15, 13, Z, Z,  11, 9, Z, Z,  7, 5, Z, Z,  3, 1, Z, Z
    );


    // back  = [r g b a | r g b a | r g b a | ...]
    // front = [r g b a | r g b a | r g b a | ...]

    __m256i back_left   = _mm256_shuffle_epi8(back,  LEFT_SPLIT);
    __m256i back_right  = _mm256_shuffle_epi8(back,  RIGHT_SPLIT);
    __m256i front_left  = _mm256_shuffle_epi8(front, LEFT_SPLIT);
    __m256i front_right = _mm256_shuffle_epi8(front, RIGHT_SPLIT);
    // back_left   = [0 r 0 g | 0 r 0 g | 0 r 0 g | ...]
    // back_right  = [0 b 0 a | 0 b 0 a | 0 b 0 a | ...]
    // front_left  = [0 r 0 g | 0 r 0 g | 0 r 0 g | ...]
    // front_right = [0 b 0 a | 0 b 0 a | 0 b 0 a | ...]

    __m256i alpha_front = _mm256_shuffle_epi8(front, ALPHA_EXTRACTOR);
    __m256i alpha_front_inverse = _mm256_sub_epi8(WHITE, alpha_front);
    // alpha_front = [0 a 0 a | 0 a 0 a | 0 a 0 a | ...]

    __m256i final_left  = _mm256_add_epi8(_mm256_mullo_epi16(front_left,  alpha_front),
                                          _mm256_mullo_epi16(back_left,   alpha_front_inverse));
    __m256i final_right = _mm256_add_epi8(_mm256_mullo_epi16(front_right, alpha_front),
                                          _mm256_mullo_epi16(back_right,  alpha_front_inverse));

    return _mm256_add_epi8(_mm256_shuffle_epi8(final_left,  ASSEMBLE_LEFT),
                           _mm256_shuffle_epi8(final_right, ASSEMBLE_RIGHT));
}

static inline void mix_colors_no_simd(sf::Uint8 foreground[4], sf::Uint8 background[4], sf::Uint8 result[4]) {
    for (int channel = 0; channel < 4; ++channel) {
        result[channel] = (sf::Uint8) (((int) foreground[channel] * foreground[3] +
            (int) background[channel] * (255 - foreground[3])) >> 8);
    }
}

void draw_scene(RenderScene* scene, sf::Shader* shader, sf::RenderWindow* window) {
    SILENCE_UNUSED(shader);

    static sf::Uint8 pixels[SCREEN_WIDTH * SCREEN_HEIGHT * 4] = "";

    const sf::Uint8* background_pixels = scene->background.getPixelsPtr();
    const sf::Uint8* foreground_pixels = scene->foreground.getPixelsPtr();

    for (unsigned __render_pass_id = 0; __render_pass_id < RENDER_WEIGHT; ++__render_pass_id) {

    ON_SIMD(
    for (size_t pixel_id = 0; pixel_id + 7 < SCREEN_WIDTH * SCREEN_HEIGHT; pixel_id += 8) {
        int pixel_x = (int) pixel_id % (int) SCREEN_WIDTH,
            pixel_y = (int) pixel_id / (int) SCREEN_WIDTH;
        
        if (pixel_y * (int) scene->background.getSize().x + pixel_x + 7 >=
            (int) scene->background.getSize().x * (int) scene->background.getSize().y) continue;

        if ((pixel_x < scene->foreground_position.x || pixel_y < scene->foreground_position.y) ||
            (pixel_x + 7 >= scene->foreground_position.x + (int) scene->foreground.getSize().x ||
            pixel_y >= scene->foreground_position.y + (int) scene->foreground.getSize().y)) {

            memcpy(pixels + pixel_id * 4, background_pixels + 
                   4 * (pixel_y * (int) scene->background.getSize().x + pixel_x), sizeof(sf::Uint8) * 4 * 8);
            continue;
        }

        __m256i color_bkg = _mm256_loadu_si256((__m256i_u*) (background_pixels +
            4 * (pixel_y * (int) scene->background.getSize().x + pixel_x)));
        __m256i color_frg = _mm256_loadu_si256((__m256i_u*) (foreground_pixels +
            4 * ((pixel_y - scene->foreground_position.y) * (int) scene->foreground.getSize().x +
            pixel_x - scene->foreground_position.x)));

        _mm256_storeu_si256((__m256i_u*) (pixels + pixel_id * 4), mix_colors(color_bkg, color_frg));
    }
    )  // End of SIMD-based section

    NO_SIMD(
    for (size_t pixel_id = 0; pixel_id < SCREEN_WIDTH * SCREEN_HEIGHT; ++pixel_id) {
        int pixel_x = (int) pixel_id % (int) SCREEN_WIDTH,
            pixel_y = (int) pixel_id / (int) SCREEN_WIDTH;
        
        if (pixel_y * (int) scene->background.getSize().x + pixel_x + 7 >=
            (int) scene->background.getSize().x * (int) scene->background.getSize().y) continue;

        if ((pixel_x < scene->foreground_position.x || pixel_y < scene->foreground_position.y) ||
            (pixel_x + 7 >= scene->foreground_position.x + (int) scene->foreground.getSize().x ||
            pixel_y >= scene->foreground_position.y + (int) scene->foreground.getSize().y)) {

            memcpy(pixels + pixel_id * 4, background_pixels + 
                   4 * (pixel_y * (int) scene->background.getSize().x + pixel_x), sizeof(sf::Uint8) * 4);
            continue;
        }

        sf::Uint8 frg_color[4] = {0, 0, 0, 0};
        memcpy(frg_color, foreground_pixels +
            4 * ((pixel_y - scene->foreground_position.y) * (int) scene->foreground.getSize().x +
            pixel_x - scene->foreground_position.x), 4 * sizeof(*frg_color));
        sf::Uint8 bkg_color[4] = {0, 0, 0, 0};
        memcpy(bkg_color, background_pixels + 4 * (pixel_y * (int) scene->background.getSize().x + pixel_x),
            4 * sizeof(*bkg_color));

        sf::Uint8* final_color = pixels + pixel_id * 4;
        mix_colors_no_simd(frg_color, bkg_color, final_color);
    }

    )  // End of non-simd section

    }  // End of additional render passes

    scene->display_texture.update(pixels);

    window->draw(scene->display_sprite);
}

void on_closure_event(sf::Event event, sf::RenderWindow* window) {
    bool condition = false;
    condition |= event.type == sf::Event::Closed;
    condition |= event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape;
    if (condition) window->close();
}

void on_graph_tick_event(sf::RenderWindow* window, sf::Shader* shader, RenderScene* scene, sf::Time dt) {
    SILENCE_UNUSED(dt);
    window->clear(sf::Color::Black);
    draw_scene(scene, shader, window);
    window->display();
}

void on_phys_tick_event(sf::RenderWindow* window, RenderScene* scene, sf::Time dt) {
    update_scene(scene, window, dt);
}
