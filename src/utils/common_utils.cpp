#include "common_utils.h"

#include <stdlib.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "lib/util/dbg/logger.h"
#include "lib/util/dbg/debug.h"

#include "lib/speaker.h"

void MemorySegment_ctor(MemorySegment* segment) {
    segment->content = (int*) calloc(segment->size, sizeof(*segment->content));
}

void MemorySegment_dtor(MemorySegment* segment) {
    free(segment->content);
    segment->content = NULL;
    segment->size = 0;
}

void _MemorySegment_dump(MemorySegment* segment, unsigned int importance) {
    for (size_t id = 0; id < segment->size; ++id) {
        _log_printf(importance, "dump", "[%6lld] = %d\n", (long long) id, segment->content[id]);
    }
}

int clamp(const int value, const int left, const int right) {
    if (value < left) return left;
    if (value > right) return right;
    return value;
}

// Amazing, do not change anything!
// Completed the owl, sorry.
void print_owl(const int argc, void** argv, const char* argument) {
    SILENCE_UNUSED(argc); SILENCE_UNUSED(argv); SILENCE_UNUSED(argument);
    printf("-Owl argument detected, dropping emergency supply of owls.\n");
    for (int index = 0; index < NUMBER_OF_OWLS; index++) {
        puts(R"(    A_,,,_A    )");
        puts(R"(   ((O)V(O))   )");
        puts(R"(  ("\"|"|"/")  )");
        puts(R"(   \"|"|"|"/   )");
        puts(R"(     "| |"     )");
        puts(R"(      ^ ^      )");
    }
}

void mute_speaker(const int argc, void** argv, const char* argument) {
    SILENCE_UNUSED(argc); SILENCE_UNUSED(argv); SILENCE_UNUSED(argument);
    speaker_set_mute(false);
}

const char* get_input_file_name(const int argc, const char** argv, const char* default_name) {
    const char* file_name = default_name;

    for (int argument_id = 1; argument_id < argc; ++argument_id) {
        if (*argv[argument_id] == '-') continue;
        file_name = argv[argument_id];
        break;
    }

    return file_name;
}

const char* get_output_file_name(const int argc, const char** argv, const char* default_name) {
    const char* file_name = default_name;

    bool enc_first_name = false;
    for (int argument_id = 1; argument_id < argc; ++argument_id) {
        if (*argv[argument_id] == '-') continue;
        file_name = argv[argument_id];
        if (enc_first_name) return file_name;
        else enc_first_name = true;
    }

    return default_name;
}

int max(int alpha, int beta) {
    return alpha > beta ? alpha : beta;
}

MmapResult map_file(const char* name, int oflag, int prot) {
    int fd = open(name, oflag);
    log_printf(STATUS_REPORTS, "status", "Mapping file %s with descriptor %d\n", name, fd);
    if (fd < 0) {
        log_dup(ERROR_REPORTS, "error", "Failed to open file: %s\n", name);
        close(fd);
        return {};
    }
    struct stat st = {};
    if (fstat(fd, &st) < 0) {
        log_dup(ERROR_REPORTS, "error", "Failed to stat file: %s\n", name);
        close(fd);
        return {};
    }
    void* map = mmap(NULL, (size_t) st.st_size, prot, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        log_dup(ERROR_REPORTS, "error", "Failed to mmap file: %s\n", name);
        close(fd);
        return {};
    }
    return (MmapResult) {.ptr = map, .fd = fd, .size = (size_t) st.st_size};
}

void std_close(void* file_descriptor_ptr) {
    log_printf(STATUS_REPORTS, "status", "Closing file descriptor %d\n", *(int*)file_descriptor_ptr);
    close(*(int*)file_descriptor_ptr);
}
