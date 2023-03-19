#include "gui_utils.h"

#include <stdlib.h>
#include <stdarg.h>

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
    scene->camera = (Camera) {.position = glm::vec3(0.0f, 1.0f, 1.0f),
                              .forward  = glm::vec3(0.0f, 0.0f, -1.0f),
                              .up       = glm::vec3(0.0f, 1.0f, 0.0f),
                              .fov = 90};
    scene->time = sf::Time::Zero;
    scene->last_update_time = sf::Time::Zero;
    scene->canvas[0] = sf::Vertex(sf::Vector2f(0.0f,                                  0.0f), sf::Color::Black,  sf::Vector2f(0.0f, 0.0f));
    scene->canvas[1] = sf::Vertex(sf::Vector2f(0.0f,                 (float) SCREEN_HEIGHT), sf::Color::Red,    sf::Vector2f(0.0f, 1.0f));
    scene->canvas[2] = sf::Vertex(sf::Vector2f((float) SCREEN_WIDTH, (float) SCREEN_HEIGHT), sf::Color::Yellow, sf::Vector2f(1.0f, 1.0f));
    scene->canvas[3] = sf::Vertex(sf::Vector2f((float) SCREEN_WIDTH,                  0.0f), sf::Color::Green,  sf::Vector2f(1.0f, 0.0f));
    scene->camera.aspect_ratio = (float) SCREEN_WIDTH / (float) SCREEN_HEIGHT;
}

void update_scene(RenderScene* scene, sf::RenderWindow* window, sf::Time dt) {
    sf::Vector2i mouse_pos = sf::Mouse::getPosition(*window);
    glm::vec2 rel_mouse_pos = glm::vec2(mouse_pos.x, mouse_pos.y) * 2.0f / glm::vec2(SCREEN_WIDTH, SCREEN_HEIGHT) - 
                              glm::vec2(1.0f, 1.0f);
    rel_mouse_pos.y *= -1.0f;
    scene->camera.position = glm::vec3(0.0f, 1.0f, 1.0f) + glm::vec3(rel_mouse_pos, 0.0f) * 0.1f;

    scene->time += dt;
    scene->last_update_time = scene->time;
}

void draw_scene(RenderScene* scene, sf::Shader* shader, sf::RenderWindow* window) {
    shader->setUniform("time", scene->time.asSeconds());
    set_camera_uniforms(scene->camera, shader);
    window->draw(scene->canvas, shader);
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
