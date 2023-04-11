# Experiment results
## Task
Measure performance of different ways of blending two images on top of each other.

Approaches:
 1. Traditional per-pixel loop
 2. Same approach, accelerated with AVX2 instruction
 3. Shader rendering *(has not been implemented)*

## Conditions
Both programs were compiled with the same version of GCC with the exact same flag (except one -D which defines program type, as this difference is measured).

Screen size = 800 x 600
Foreground and background files are located in folder 'assets/images/' and named accordingly

128 rendering ticks per draw tick

## Results
Single-register RPS (renders per second): 78 (varying from 76 to 80)

SIMD RPS (renders per second): stable 365 (varying from 340 to 382).

## Result processing
SIMD has increased performance of the program by 4.25 - 5.03 times.
