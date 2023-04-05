/**
 * @file config.h
 * @author Kudryashov Ilya (kudriashov.it@phystech.edu)
 * @brief List of constants used inside the main program.
 * @version 0.1
 * @date 2022-11-01
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef MAIN_CONFIG_H
#define MAIN_CONFIG_H

#include <stdlib.h>

static const int NUMBER_OF_OWLS = 10;
static const unsigned int DFLT_TOKEN_COUNT = 1024;
static const size_t MAX_CALL_LENGTH = 1024;

static const char DFLT_PATCH_NAME[] = "patch.dat";
static const char DFLT_PROGRAM_NAME[] = "CRACKME.COM";

static const unsigned SCREEN_WIDTH = 800;
static const unsigned SCREEN_HEIGHT = 600;

#define BACKGROUND_NAME "images/background.png"
#define FOREGROUND_NAME "images/foreground.png"

#endif