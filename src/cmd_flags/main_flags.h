/**
 * @file frontend_flags.h
 * @author Kudryashov Ilya (kudriashov.it@phystech.edu)
 * @brief Flags for the main game program.
 * @version 0.1
 * @date 2022-12-14
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "common_flags.h"

{ {'T', ""}, { GET_WRAPPER(max_tokens), 1, edit_int },
    "set max number of tokens in patch file.\n"
    "\tDoes not check if integer was specified." },