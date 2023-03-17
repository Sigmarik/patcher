#include "patch_parser.h"

#include "lib/util/dbg/logger.h"

#define CERROR(...) do {                                            \
    log_dup(ERROR_REPORTS, "error", "At symbol %d (line %d)\n",     \
            (int)(*token_ptr)->position, (int)(*token_ptr)->line);  \
    log_dup(ERROR_REPORTS, "error", __VA_ARGS__);                   \
}while (0)

static bool isnum16(char symbol) {
    return ('0' <= symbol && symbol <= '9') || ('A' <= symbol && symbol <= 'F');
}

static unsigned char to_num(char symbol) {
    if ('0' <= symbol && symbol < '9')
        return (unsigned char) (symbol - '0');
    return (char)10 + symbol - 'A';
}

void tokenize(const char* const content, PatchToken* buffer, size_t buffer_length) {
    unsigned comment_depth = 0;
    size_t line = 1;
    const char* line_start = content;
    PatchToken* current_token = buffer;
    for (const char* cur = content; *cur != '\0' && current_token < buffer + buffer_length - 1; cur++) {
        switch(*cur) {
            case '\n': {++line; line_start = cur + 1; break;}
            case ';': {++comment_depth; break;}
            case ':': {if (comment_depth) --comment_depth; break;}
            default: break;
        }

        if (comment_depth) continue;

        if (isnum16(cur[0]) && isnum16(cur[1])){
            *(current_token++) = (PatchToken) {
                .line = line,
                .position = (uintptr_t)cur - (uintptr_t)line_start,
                .value = to_num(cur[0]) * 16 + to_num(cur[1]),
                .type = TOK_BYTE,
            };
            ++cur;
            continue;
        }

        switch (*cur) {
            case '[': {*(current_token++) = (PatchToken) {
                .line = line,
                .position = (uintptr_t)cur - (uintptr_t)line_start,
                .value = 0,
                .type = TOK_OP_BRACKET,
            }; break;}
            case ']': {*(current_token++) = (PatchToken) {
                .line = line,
                .position = (uintptr_t)cur - (uintptr_t)line_start,
                .value = 0,
                .type = TOK_CL_BRACKET,
            }; break;}
            default: break;
        }
    }

    *current_token = (PatchToken) {.line = line, .position = 0, .value = 0, .type = TOK_END};

    return;
}

static int read_command(const PatchToken** token_ptr, char* file) {
    if ((*token_ptr)->type != TOK_OP_BRACKET) {
        CERROR("Patch instruction expected.\n");
        return 1;
    }

    ++*token_ptr;

    uintptr_t address = 0;
    while ((*token_ptr)->type == TOK_BYTE) {
        address <<= 8;
        address += (*token_ptr)->value;
        ++*token_ptr;
    }

    if ((*token_ptr)->type != TOK_CL_BRACKET) {
        CERROR("Closing bracket expected.\n");
        return 1;
    }

    ++*token_ptr;

    while ((*token_ptr)->type == TOK_BYTE) {
        log_printf(STATUS_REPORTS, "status", "Patching byte at address %04X with byte %02X\n", 
            (unsigned int) address, (*token_ptr)->value);
        file[address++] = (*token_ptr)->value;
        ++*token_ptr;
    }

    return 0;
}

void execute(const PatchToken* tokens, char* file) {
    while (tokens->type != TOK_END) {
        if (read_command(&tokens, file)) break;
    }
}