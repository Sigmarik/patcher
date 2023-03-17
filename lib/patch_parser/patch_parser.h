/**
 * @file tokenizer.h
 * @author Ilya Kudryashov (kudriashov.it@phystech.edu)
 * @brief Tokenizer module for parsing .dat patch files.
 * @version 0.1
 * @date 2023-03-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdlib.h>
#include <inttypes.h>

enum TOKEN_TYPE {
    TOK_END,
    TOK_OP_BRACKET,
    TOK_CL_BRACKET,
    TOK_BYTE,
};

static const char* TOKEN_TYPE_NAMES[] = {
    "END",
    "OP_BRACKET",
    "CL_BRACKET",
    "BYTE",
};

struct PatchToken {
    size_t line = 0;
    size_t position = 0;
    unsigned char value = 0;
    TOKEN_TYPE type = TOK_BYTE;
};

/**
 * @brief Tokenize the content of the file
 * 
 * @param content content of the file
 * @param buffer [out] buffer to fill with tokens
 * @param buffer_length max available length of the buffer
 */
void tokenize(const char* const content, PatchToken* buffer, size_t buffer_length);

/**
 * @brief Apply tokenized changes to the file.
 * 
 * @param tokens list of tokens to apply (should end with TOK_END token)
 * @param file [out] file to apply changes to
 */
void execute(const PatchToken* tokens, char* file);

#endif