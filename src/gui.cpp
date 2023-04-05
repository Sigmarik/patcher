/**
 * @file main.cpp
 * @author Ilya Kudryashov (kudriashov.it@phystech.edu)
 * @brief Main patcher program.
 * @version 0.1
 * @date 2023-03-14
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <ctype.h>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include "lib/util/dbg/debug.h"
#include "lib/util/argparser.h"
#include "lib/alloc_tracker/alloc_tracker.h"
#include "lib/util/util.h"

#include "utils/config.h"
#include "utils/gui_utils.h"

#define GUI

int main(const int argc, const char** argv) {
    atexit(log_end_program);

    start_local_tracking();

    unsigned int log_threshold = STATUS_REPORTS;
    MAKE_WRAPPER(log_threshold);

    ActionTag line_tags[] = {
        #include "cmd_flags/gui_flags.h"
    };
    const int number_of_tags = ARR_SIZE(line_tags);

    parse_args(argc, argv, number_of_tags, line_tags);
    log_init("program_log.html", log_threshold, &errno);
    print_label();

    char patcher_call[MAX_CALL_LENGTH] = PATCHER_NAME;
    char* word_ptr = patcher_call + strlen(patcher_call);
    for (size_t word_id = 1; word_id < (size_t) argc; ++word_id) {
        *(word_ptr++) = ' ';
        strcpy(word_ptr, argv[word_id]);
        word_ptr += strlen(argv[word_id]);
    }

    int patch_result = system(patcher_call);
    if (patch_result != 0) {
        log_dup(ERROR_REPORTS, "error", "Patch failed!\n");
    }

    if (sf::Shader::isAvailable() == false) {
        log_dup(ERROR_REPORTS, "error", "Shader support is required to see the animation! (the program was patched though)\n");
        return_clean(EXIT_FAILURE);
    }

    sf::Shader shader;
    if (!shader.loadFromFile("shaders/textured.frag", sf::Shader::Fragment)) {
        log_dup(ERROR_REPORTS, "error", "Failed to load shader!\n");
        return_clean(EXIT_FAILURE);
    }

    sf::Clock ticker_clock;

    RenderScene scene = {};
    init_scene(&scene);

    track_allocation(scene, scene_dtor);

    sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Cracked y'all!");

    while (window.isOpen()) {
        sf::Time delta_time = ticker_clock.getElapsedTime();
        ticker_clock.restart();

        static char title[128] = "";
        sprintf(title, "Cracked y'all! [FPS: %d]",
            (int) (1.0 / delta_time.asSeconds()));

        window.setTitle(std::string(title));

        sf::Event event;
        while (window.pollEvent(event)) {
            on_closure_event(event, &window);
        }

        on_phys_tick_event(&window, &scene, delta_time);
        on_graph_tick_event(&window, &shader, &scene, delta_time);
    }

    return_clean(errno == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
