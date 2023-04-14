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

SIMD RPS (renders per second): 361 (varying from 340 to 382).

## Result processing
SIMD has increased performance of the program by 4.25 - 5.03 times.

## ASM code analysis
Here is a code sample taken from *godbolt.org*, featuring assembly code of alpha blending function with and without SIMD

No-SIMD alpha blending function:
```
mix_colors_no_simd(unsigned char*, unsigned char*, unsigned char*):
        mov     r9, rdi
        xor     eax, eax
        mov     r10d, 255
.L2:
        movzx   r8d, BYTE PTR [r9+3]
        movzx   ecx, BYTE PTR [rsi+rax]
        mov     edi, r10d
        sub     edi, r8d
        imul    ecx, edi
        movzx   edi, BYTE PTR [r9+rax]
        imul    edi, r8d
        add     ecx, edi
        mov     BYTE PTR [rdx+rax], ch
        add     rax, 1
        cmp     rax, 4
        jne     .L2
        ret
```

SIMD-based alpha blending function:
```
mix_colors(long long __vector(4), long long __vector(4)):
        vmovdqa ymm3, YMMWORD PTR .LC0[rip]
        vmovdqa ymm2, YMMWORD PTR .LC1[rip]
        vpshufb ymm5, ymm0, ymm3
        vpshufb ymm4, ymm1, ymm3
        vpshufb ymm0, ymm0, ymm2
        vpshufb ymm3, ymm1, ymm2
        vmovdqa ymm2, YMMWORD PTR .LC3[rip]
        vpshufb ymm1, ymm1, YMMWORD PTR .LC2[rip]
        vpmullw ymm3, ymm1, ymm3
        vpsubb  ymm2, ymm2, ymm1
        vpmullw ymm1, ymm1, ymm4
        vpmullw ymm0, ymm2, ymm0
        vpmullw ymm2, ymm2, ymm5
        vpaddb  ymm0, ymm0, ymm3
        vpaddb  ymm1, ymm1, ymm2
        vpshufb ymm0, ymm0, YMMWORD PTR .LC4[rip]
        vpshufb ymm1, ymm1, YMMWORD PTR .LC5[rip]
        vpaddb  ymm0, ymm0, ymm1
        ret
.LC0: [LEFT_SPLIT      vector, defined as sequence of .byte-s]
.LC1: [RIGHT_SPLIT     vector, defined as sequence of .byte-s]
.LC2: [ALPHA_EXTRACTOR vector, defined as sequence of .byte-s]
.LC3: [WHITE           vector, defined as sequence of .byte-s]
.LC4: [ASSEMBLE_RIGHT  vector, defined as sequence of .byte-s]
.LC5: [ASSEMBLE_LEFT   vector, defined as sequence of .byte-s]
```

Do keep in mind, that vectors under labels were written as sequences of bytes like following (LEFT_SPLIT vector):
```
.byte   0
.byte   -128
.byte   1
.byte   -128
.byte   4
.byte   -128
.byte   5
.byte   -128
.byte   8
.byte   -128
.byte   9
.byte   -128
.byte   12
.byte   -128
.byte   13
.byte   -128
.byte   16
.byte   -128
.byte   17
.byte   -128
.byte   20
.byte   -128
.byte   21
.byte   -128
.byte   24
.byte   -128
.byte   25
.byte   -128
.byte   28
.byte   -128
.byte   29
.byte   -128
```

As we can see, the main acceleration happens because of the number of pixels each function processes at once. SIMD-based function simultaneously processes 8 pixels, while its counterpart can only fully process one pixel while taking double the time it takes SIMD-based function to execute.

However, blending function takes only part of that RPS growth shown in the `results` section. The main advantage of using SIMD instructions may only come in loop optimization, as with SIMDs program iterates through portions of 8 pixels, thus, copying larger portions of data from background to the screen in one call compared to single pixel at a time with classic approach.
