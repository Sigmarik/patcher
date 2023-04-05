# Experiment results
## Task
Measure performance of different ways of rendering Mandelbrot set on the user's screen.

Approaches:
 1. Traditional per-pixel loop
 2. Same approach, accelerated with AVX2 instruction
 3. Shader rendering *(has not been implemented)*

## Conditions
Both programs were compiled with the same version of GCC with the exact same flag (except one -D which defines program type, as this difference is measured).

Screen size = 800 x 600

Mandelbrot set with 256 iterations, kill distance of 10

Magnification = 1 / (0.5 + 500) (central zoom with X size of the screen translating to 3 scene units with default ( = 1) magnification)

X pan = -1.48

Experiment set indicator: color of the fog (proven not to affect performance)

128 rendering ticks per draw tick

## Results
Single-register RPS (renders per second): 3-4 (was assumed to be 4 +- 1 in calculations)

SIMD RPS (renders per second): stable 23 (23.5 +- 0.5)

## Result processing
SIMD increased performance of the program by around 6.3 +- 1.7 times.
