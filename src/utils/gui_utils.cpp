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

void update_scene(RenderScene* scene, sf::RenderWindow* window, sf::Time dt) {
    sf::Vector2i mouse_pos = sf::Mouse::getPosition(*window);
    glm::vec2 rel_mouse_pos = glm::vec2(mouse_pos.x, mouse_pos.y) * 2.0f / glm::vec2(SCREEN_WIDTH, SCREEN_HEIGHT) - 
                              glm::vec2(1.0f, 1.0f);
    rel_mouse_pos.y *= -1.0f;

    scene->foreground_position = mouse_pos;

    scene->time += dt;
    scene->last_update_time = scene->time;
}

void draw_scene(RenderScene* scene, sf::Shader* shader, sf::RenderWindow* window) {
    __m256i shifter = _mm256_set_epi32(0, 1, 2, 3, 4, 5, 6, 7);

    static sf::Uint8 pixels[SCREEN_WIDTH * SCREEN_HEIGHT * 4] = "";
    unsigned pixel_buffer[8 * 4] = {};

    const sf::Uint8* background_pixels = scene->background.getPixelsPtr();
    const sf::Uint8* foreground_pixels = scene->foreground.getPixelsPtr();

    for (size_t pixel_id = 0; pixel_id + 7 < SCREEN_WIDTH * SCREEN_HEIGHT; pixel_id += 8) {
        int pixel_x = (int) pixel_id % (int) SCREEN_WIDTH,
            pixel_y = (int) pixel_id / (int) SCREEN_WIDTH;
        
        if (pixel_x < scene->foreground_position.x || pixel_y < scene->foreground_position.y) continue;
        if (pixel_x + 7 >= scene->foreground_position.x + scene->foreground.getSize().x ||
            pixel_y >= scene->foreground_position.y + scene->foreground.getSize().y) continue;

        __m256i local_x = _mm256_set1_epi32(pixel_x),
                local_y = _mm256_set1_epi32(pixel_y);
        
        local_x = _mm256_sub_epi32(local_x, shifter);

        __m256i color_bkg = _mm256_loadu_epi8(background_pixels +
            pixel_y * scene->background.getSize().x + pixel_x);
        __m256i color_frg = _mm256_loadu_epi8(foreground_pixels +
            (pixel_y - scene->foreground_position.y) * scene->foreground.getSize().x +
            pixel_x - scene->foreground_position.x);

        _mm256_storeu_epi8(pixels + pixel_id * 4, color_frg);

        memcpy(pixels + pixel_id * 4, pixel_buffer, sizeof(pixel_buffer));
    }

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
