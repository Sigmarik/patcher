/**
 * @file common.h
 * @author Kudryashov Ilya (kudriashov.it@phystech.edu)
 * @brief Utility features useful in all programs.
 * @version 0.1
 * @date 2022-10-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

#include <cstring>
#include <ctype.h>

#include "config.h"
#include "lib/alloc_tracker/alloc_tracker.h"

/**
 * @brief Array with stored size.
 * 
 */
struct MemorySegment {
    int* content = NULL;
    size_t size = 1024;
};

void MemorySegment_ctor(MemorySegment* segment);
void MemorySegment_dtor(MemorySegment* segment);

#define MemorySegment_dump(segment, importance) do { \
    log_printf(importance, "dump", "Memory segment at %p. Size: %ld Content: %p\n", segment, (segment)->size, (segment)->content); \
    _MemorySegment_dump(segment, importance); \
} while (0)

void _MemorySegment_dump(MemorySegment* segment, unsigned int importance);

/**
 * @brief Limit specified value between left and right border.
 * 
 * @param value variable to check
 * @param left left border
 * @param right right border
 * @return 
 */
int clamp(const int value, const int left, const int right);

/**
 * @brief Print a bunch of owls.
 * 
 * @param argc unimportant
 * @param argv unimportant
 * @param argument unimportant
 */
void print_owl(const int argc, void** argv, const char* argument);

/**
 * @brief Disable all voice lines.
 * 
 * @param argc unimportant
 * @param argv unimportant
 * @param argument unimportant
 */
void mute_speaker(const int argc, void** argv, const char* argument);

/**
 * @brief Get the input file name from the list of command line arguments.
 * 
 * @param argc argument count
 * @param argv argument values
 * @param default_name default file name
 * @return const char* 
 */
const char* get_input_file_name(const int argc, const char** argv, const char* default_name);

/**
 * @brief Get the output file name from the list of command line arguments.
 * 
 * @param argc argument count
 * @param argv argument values
 * @param default_name default file name
 * @return const char* 
 */
const char* get_output_file_name(const int argc, const char** argv, const char* default_name);

/**
 * @brief Get maximum of two values.
 * 
 * @param alpha 
 * @param beta 
 * @return maximum of alpha and beta
 */
int max(int alpha, int beta);

/**
 * @brief Memory mapping result.
 * 
 * @param ptr pointer to the mapped area
 * @param fd file descriptor
 */
struct MmapResult {
    void* ptr = NULL;
    int fd = 0;
    size_t size = 0;
};

/**
 * @brief Memory map file and return pointer to the mapped area.
 * 
 * @param name file name
 * @param oflags open flags
 * @param prot mmap protection flags
 * @return MapResult
*/
MmapResult map_file(const char* name, int oflags, int prot);

/**
 * @brief Standard wrapper for close() function
 * 
 * @param file_descriptor 
 */
void std_close(void* file_descriptor_ptr);

#define MAKE_WRAPPER(name) void* __wrapper_##name[] = {&name}

/**
 * @brief Read user input and do actions depending on if user entered yes or no.
 * 
 * @param action_yes code to execute on YES
 * @param action_no code to execute on NO
 */
#define yn_branch(action_yes, action_no) do {                               \
    char __answer = '\0';                                                   \
    scanf(" %c", &__answer);                                                \
    __answer = (char)tolower(__answer);                                     \
                                                                            \
    while (getc(stdin) != '\n');                                            \
                                                                            \
    if (__answer == 'y') {                                                  \
        log_printf(STATUS_REPORTS, "status", "User answered with YES.\n");  \
        action_yes;                                                         \
        break;                                                              \
    }                                                                       \
    if (__answer == 'n') {                                                  \
        log_printf(STATUS_REPORTS, "status", "User answered with NO.\n");   \
        action_no;                                                          \
        break;                                                              \
    }                                                                       \
    printf("yes/no expected, try again.\n>>> ");                            \
} while(true)

#endif