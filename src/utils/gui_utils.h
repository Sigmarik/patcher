/**
 * @file gui_utils.h
 * @author Ilya Kudryashov (kudriashov.it@phystech.edu)
 * @brief Utility functions for GUI program
 * @version 0.1
 * @date 2023-03-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef GUI_UTILS_H
#define GUI_UTILS_H

#include "common_utils.h"

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <glm/glm.hpp>

/**
 * @brief Print program label and build date/time to console and log.
 * 
 */
void print_label();

struct Camera {
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 forward  = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up       = glm::vec3(0.0f, 1.0f, 0.0f);
    float fov = 90.0f;
    float aspect_ratio = 1.0f;
};

void set_camera_uniforms(const Camera camera, sf::Shader* shader);

struct RenderScene {
    Camera camera = (Camera) {};
    sf::Time time = sf::Time::Zero;
    sf::Time last_update_time = sf::Time::Zero;
    sf::VertexArray canvas = sf::VertexArray(sf::Quads, 4);
};

void init_scene(RenderScene* scene);

void update_scene(RenderScene* scene, sf::RenderWindow* window, sf::Time dt);

void draw_scene(RenderScene* scene, sf::Shader* shader, sf::RenderWindow* window);

/**
 * @brief Process window closure event
 * 
 * @param event 
 * @param window 
 */
void on_closure_event(sf::Event event, sf::RenderWindow* window);

/**
 * @brief Process graphics tick event
 * 
 * @param window 
 * @param shader 
 * @param dt delta time
 */
void on_graph_tick_event(sf::RenderWindow* window, sf::Shader* shader, RenderScene* scene, sf::Time dt);

/**
 * @brief Process physics tick event
 * 
 * @param window 
 * @param scene 
 * @param dt 
 */
void on_phys_tick_event(sf::RenderWindow* window, RenderScene* scene, sf::Time dt);

#endif