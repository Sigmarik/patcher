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
    scene->texture.create(SCREEN_WIDTH, SCREEN_HEIGHT);
    scene->sprite = sf::Sprite(scene->texture);
    scene->camera.aspect_ratio = (float) SCREEN_WIDTH / (float) SCREEN_HEIGHT;
}

void update_scene(RenderScene* scene, sf::RenderWindow* window, sf::Time dt) {
    sf::Vector2i mouse_pos = sf::Mouse::getPosition(*window);
    glm::vec2 rel_mouse_pos = glm::vec2(mouse_pos.x, mouse_pos.y) * 2.0f / glm::vec2(SCREEN_WIDTH, SCREEN_HEIGHT) - 
                              glm::vec2(1.0f, 1.0f);
    rel_mouse_pos.y *= -1.0f;
    scene->camera.position = glm::vec3(0.0f, 1.0f, 2.0f) + glm::vec3(rel_mouse_pos, 0.0f) * 0.1f;

    scene->time += dt;
    scene->last_update_time = scene->time;
}

void draw_scene(RenderScene* scene, sf::Shader* shader, sf::RenderWindow* window) {
    float animation_stage = (float) sin((double) scene->time.asSeconds() * ANIMATION_SPEED) / 2.0f + 0.5f;
    float scale = 1.0f / (0.5f + ANIMATION_BRANCH(animation_stage, 1.0) * MAX_MAGNIFICATION);

    const __m256 shifter = _mm256_mul_ps(_mm256_set_ps(
        0 * SCENE_SCALE, 
        1 * SCENE_SCALE, 
        2 * SCENE_SCALE, 
        3 * SCENE_SCALE, 
        4 * SCENE_SCALE, 
        5 * SCENE_SCALE, 
        6 * SCENE_SCALE, 
        7 * SCENE_SCALE), _mm256_set1_ps(scale));

    const __m256 DEATH_DISTANCE = _mm256_set1_ps(POINT_DEATH_DISTANCE * POINT_DEATH_DISTANCE);

    static sf::Uint8 pixels[SCREEN_WIDTH * SCREEN_HEIGHT * 4] = "";
    static unsigned pixel_buffer[8] = {};

    for (unsigned evaluation_pass_id = 0; evaluation_pass_id < EVALUATION_WEIGHT; ++evaluation_pass_id) {

    ON_SIMD(
    for (size_t pixel_id = 0; pixel_id + 7 < SCREEN_WIDTH * SCREEN_HEIGHT; pixel_id += 8) {
        int pixel_x = (int) pixel_id % (int) SCREEN_WIDTH - (int) SCREEN_WIDTH / 2,
            pixel_y = (int) pixel_id / (int) SCREEN_WIDTH - (int) SCREEN_HEIGHT / 2;

        __m256 local_x = _mm256_set1_ps((float) pixel_x * SCENE_SCALE * scale + SCENE_X_PAN),
               local_y = _mm256_set1_ps((float) pixel_y * SCENE_SCALE * scale);
        
        local_x = _mm256_sub_ps(local_x, shifter);
        
        __m256 current_x = local_x, current_y = local_y;
        __m256i iter_count = _mm256_set1_epi32(0);

        for (unsigned iter_id = 0; iter_id < RENDER_ITERATION_COUNT; ++iter_id) {
            __m256 previous_x = current_x, previous_y = current_y;
            current_x = _mm256_add_ps(_mm256_sub_ps(
                _mm256_mul_ps(previous_x, previous_x),
                _mm256_mul_ps(previous_y, previous_y)), local_x);
            __m256 x_by_y = _mm256_mul_ps(previous_x, previous_y);
            current_y = _mm256_add_ps(_mm256_add_ps(x_by_y, x_by_y), local_y);

            __m256 distance = _mm256_add_ps(_mm256_mul_ps(current_x, current_x), _mm256_mul_ps(current_y, current_y));
            __m256i condition = (__m256i) _mm256_cmp_ps(DEATH_DISTANCE, distance, 14);
            condition = _mm256_abs_epi32(condition);
            iter_count = _mm256_add_epi32(iter_count, condition);

            if (_mm256_movemask_epi8(condition)) break;
        }

        _mm256_storeu_si256((__m256i*) pixel_buffer, iter_count);

        for (unsigned id = 0; id < 8; id++) {
            sf::Uint8 result = (sf::Uint8) sqrt(pixel_buffer[id] * 256);
            pixels[(pixel_id + id) * 4 + 0] = result;
            pixels[(pixel_id + id) * 4 + 1] = result / 2;
            pixels[(pixel_id + id) * 4 + 2] = 0;
            pixels[(pixel_id + id) * 4 + 3] = 255;
        }
    }
    )  // END OF SIMD SECTION

    NO_SIMD(
    for (size_t pixel_id = 0; pixel_id < SCREEN_WIDTH * SCREEN_HEIGHT; ++pixel_id) {
        int pixel_x = (int) pixel_id % (int) SCREEN_WIDTH - (int) SCREEN_WIDTH / 2,
            pixel_y = (int) pixel_id / (int) SCREEN_WIDTH - (int) SCREEN_HEIGHT / 2;

        float local_x = (float) pixel_x * SCENE_SCALE * scale + SCENE_X_PAN,
              local_y = (float) pixel_y * SCENE_SCALE * scale;
        
        float current_x = local_x, current_y = local_y;
        unsigned char iter_count = 0;

        for (unsigned iter_id = 0; iter_id < RENDER_ITERATION_COUNT; ++iter_id) {
            float previous_x = current_x, previous_y = current_y;
            current_x = (previous_x * previous_x) - (previous_y * previous_y) + local_x;
            current_y = 2 * previous_x * previous_y + local_y;

            float distance = current_x * current_x + current_y * current_y;
            if (distance > POINT_DEATH_DISTANCE * POINT_DEATH_DISTANCE) break;

            iter_count++;
        }

        sf::Uint8 result = (sf::Uint8) sqrt(iter_count * 256);
        pixels[pixel_id * 4 + 0] = 0;
        pixels[pixel_id * 4 + 1] = result / 2;
        pixels[pixel_id * 4 + 2] = result;
        pixels[pixel_id * 4 + 3] = 255;
    }
    )  // END OF NO-SIMD SECTION

    }  // REPEATED SEGMENT END

    scene->texture.update(pixels);

    window->draw(scene->sprite);
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
