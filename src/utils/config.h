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

static const unsigned int RENDER_ITERATION_COUNT = 256;
static const float SCENE_SCALE = 3.0f / (float) SCREEN_WIDTH;
static const float SCENE_X_PAN = -1.48f;
static const float POINT_DEATH_DISTANCE = 10;
static const float MAX_MAGNIFICATION = 500.0f;
static const float ANIMATION_SPEED = 0.2f;

static const unsigned int EVALUATION_WEIGHT = 128;

#ifdef _DEBUG
#define ANIMATION_BRANCH(positive, negative) negative
#else
#define ANIMATION_BRANCH(positive, negative) positive
#endif

#ifdef USE_SIMD
#define ON_SIMD(...) __VA_ARGS__
#define NO_SIMD(...)
#else
#define ON_SIMD(...)
#define NO_SIMD(...) __VA_ARGS__
#endif

#endif