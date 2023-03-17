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

#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "lib/util/dbg/debug.h"
#include "lib/util/argparser.h"
#include "lib/alloc_tracker/alloc_tracker.h"
#include "lib/util/util.h"
#include "lib/patch_parser/patch_parser.h"

#include "utils/config.h"
#include "utils/main_utils.h"

#define MAIN

int main(const int argc, const char** argv) {
    atexit(log_end_program);

    start_local_tracking();

    unsigned int log_threshold = STATUS_REPORTS;
    MAKE_WRAPPER(log_threshold);

    unsigned int max_tokens = DFLT_TOKEN_COUNT;
    MAKE_WRAPPER(max_tokens);

    ActionTag line_tags[] = {
        #include "cmd_flags/main_flags.h"
    };
    const int number_of_tags = ARR_SIZE(line_tags);

    parse_args(argc, argv, number_of_tags, line_tags);
    log_init("program_log.html", log_threshold, &errno);
    print_label();

    const char* patch_file_name = get_input_file_name(argc, argv, DFLT_PATCH_NAME);
    const char* program_name = get_output_file_name(argc, argv, DFLT_PROGRAM_NAME);

    MmapResult patch_map = map_file(patch_file_name, O_RDONLY, PROT_READ);
    if (patch_map.ptr == NULL) return_clean(EXIT_FAILURE);

    MmapResult program_map = map_file(program_name, O_RDWR, PROT_READ | PROT_WRITE);
    if (program_map.ptr == NULL) return_clean(EXIT_FAILURE);

    static PatchToken* tokens = (PatchToken*) calloc(max_tokens, sizeof(*tokens));
    if (!tokens) {
        log_dup(ERROR_REPORTS, "error", "Failed to allocate space for token list.\n");
        errno = ENOMEM;
        return_clean(EXIT_FAILURE);
    }

    tokenize((const char*) patch_map.ptr, tokens, max_tokens);

    for (PatchToken* token = tokens; token < tokens + max_tokens && token->type != TOK_END; ++token) {
        log_printf(STATUS_REPORTS, "status", "Token at %p: %s (at line %ld, pos. %ld)\n", token,
            TOKEN_TYPE_NAMES[token->type], (long)token->line, (long)token->position);
    }

    execute(tokens, (char*) program_map.ptr);

    msync(program_map.ptr, program_map.size, MS_SYNC);

    return_clean(errno == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
