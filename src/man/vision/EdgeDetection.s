/* -*- mode: asm; indent-tabs-mode: nil -*- */
.intel_syntax noprefix

.globl _sobel_operator

.section .data

.equiv  imgWd, 320
.equiv  imgHt, 240
.equiv  xPitch, 2
.equiv  yPitch, imgWd * 2
.equiv  top, 0
.equiv  mid, yPitch
.equiv  bot, yPitch * 2
.equiv  nxt, yPitch * 3

.equiv  sqMag, 0
.equiv  xGrad, imgHt * yPitch
.equiv  yGrad, imgHt * yPitch * 2

.section .text

.macro Y_GRAD phase, reg
        ## Load the lower row
        ## mm\phase: | y13 | y12 | y11 | y10 |
        movq    mm\phase, [esi + bot + (\phase * 4)] # Two rows after top row
        movq    mm\reg, mm\phase

        ## Compute the difference between the rows
        ## mm\reg: | y3 | y2 | y1 | y0 | diffs
        psubsw  mm\reg, qword ptr[esi + top + (\phase * 4)]

        ## mm2 after pshufw: | y0 | y3 | y2 | y1 |
        pshufw  mm2, mm\reg, 0b00111001

        ## mm2 after addition:
        ##       | y0 + y3 | y3 + y2 | y2 + y1 | y0 + y1 |
        paddw   mm2, mm\reg

        ## mm\reg after 2nd pshufw:
        ##      | y0 + y1 | y0 + y3 | y3 + y2 | y2 + y1 |
        pshufw  mm\reg, mm2, 0b00111001

        ## mm\reg:      | xxx | xxx | y1 + 2*y2 +  y3 | y2 + 2*y1 + y0 |
        paddw   mm\reg, mm2
.endm

.macro X_GRAD phase, reg
        movq    mm0, [esi + top]
        movq    mm1, [esi + mid]
        movq    mm2, [esi + bot]

        ## bottom row in mm\phase
        ## Add middle to top, twice
        paddw   mm0, mm1
        paddw   mm0, mm1

        ## Add top to accumulator
        paddw   mm\phase, [esi + top + (\phase * 4)]

        .ifeq (\phase -1)
        movq    mm\reg, mm0
        punpckldq mm0, mm1
        punpckhdq mm\reg, mm1
        psubw  mm\reg, mm0
        movntq  [edi + xGrad], mm\reg
        .endif
.endm

        ## _sobel_operator(uint16_t *yimg, int16_t  *outX, int16_t *outY, uint16_t *mag)
_sobel_operator:
        push    ebp
        mov     ebp, esp

        push    esi
        push    edi
        push    ebx

        sub     esp, 4

        ## Load arguments into registers

        ## We have to move destination registers to the ends of the next row
        ##
        ## | o | o | o | <- source comes from this row
        ## | o | Q | o | <- destination needs to write to this row,
        ## | o | o | o | plus one byte forward for 'Q' gradient value
        mov     esi, [ebp+8]

        mov     edi, [ebp+12]
        add     edi, yPitch - 2 # Adjust destination pointer

        # Actually only does from top row through third to bottom
        mov     ebx, 238
yLoop:
        ## 4 pixels processed each iteration (320 per row / 4 = 80 iterations)
        mov     ecx, 80
xLoop:
        ## Load rows into registers to save memory accesses
	movq    mm0, [esi + top]
        movq    mm1, [esi + mid]
        movq    mm2, [esi + bot]

        ## Fetch next row into L1 cache and adjust source pointer
        prefetch [esi + nxt]
        add     esi, 8

        ## X GRADIENT calculation
	# mm3 = | - | - | z3 | z2| from previous iteration.
	# Each z is top + 2 * middle + bottom
        paddw   mm1, mm1
        paddw   mm1, mm0
        paddw   mm1, mm2
        punpckldq mm3, mm1
        psubw   mm3, mm1
        movntq [edi + xGrad], mm3

        ## Set up x sums for next iteration
        # mm3 = | - | - | z7 | z6 |
        pshufw  mm3, mm1, 0b01001110

        ## Y GRADIENT calculation
	# mm4 = | z3 | z2 | - | - | from previous iteration.
	# Each z is top - bottom
	psubw	mm0, mm2		# mm0 = | z7 | z6 | z5 | z4 |
	pshufw	mm2, mm4, 0b01001110	# mm2 = |  - |  - | z3 | z2 |
	punpckldq mm2, mm0		# mm2 = | z5 | z4 | z3 | z2 |
	paddw	mm2, mm0		# mm2 = | z5 + z7 | z4 + z6 | z3 + z5 | z2 + z4 |
	movq	mm5, mm4		# mm5 = | z3 | z2 |  - |  - |
	psrlq	mm5, 48			# mm5 = |  0 |  0 |  0 | z3 |
	movq	mm4, mm0		# mm4 = | z7 | z6 |  - |  - | for next iteration
	psllq	mm0, 16			# mm0 = | z6 | z5 | z4 |  0 |
	por	mm0, mm5		# mm0 = | z6 | z5 | z4 | z3 |
	paddw	mm0, mm0		# mm0 = | 2*z6 | 2*z5 | 2*z4 | 2*z3 |
	paddw	mm0, mm2		# mm0 = | z5 + 2*z6 + z7 | z4 + 2*z5 + z6 | z3 + 2*z4 + z5 | z2 + 2*z3 + z4 |
	movntq	[edi + yGrad], mm0

        ##
        ##
        ## MAGNITUDE CALCULATION
        ##
        ##
magnitude:
        ## movq    mm0, mm6        # mm6 = abs(x gradient), sign bit in mm0
        ## psraw   mm0, 15
        ## pxor    mm6, mm0
        ## psubw   mm6, mm0

        ## movq    mm1, mm3        # mm3 = abs(y gradient), sign bit in mm1
        ## psraw   mm1, 15
        ## pxor    mm3, mm1
        ## psubw   mm3, mm1

        ## paddw   mm1, mm1        # combine x and y signs ; range 0 to -3
        ## paddw   mm0, mm1

        ## movq    mm1, mm6        # Compare x and y gradient magnitudes
        ## pcmpgtw mm1, mm3
        ## paddw   mm0, mm0        # combine x >= y bit with sign bits to make octant
        ## paddw   mm0, mm1
        ## packsswb mm0, mm7       # write octants (bytes); range 0 to -7
        ## ## movd [octant], mm0

        ## psraw   mm6, 2          # make x and y gradient magnitudes fit in 8 bits
        ## psraw   mm3, 2
        ## movq    mm0, mm6        # mm0 gets larger magnitude
        ## pmaxsw  mm0, mm3
        ## movq    mm1, mm6        # mm1 gets smaller magnitude
        ## pminsw  mm1, mm3
        ## packsswb mm0, mm1       # write magnitudes (tangent of angle within octant)
        ## ## movq [tangent], mm0

        ## pmullw  mm6, mm6        # compute and write squared magnitude
        ## pmullw  mm3, mm3
        ## pavgw   mm6, mm3        # average is used to avoid 16-bit overflow
        ## movntq  [edi], mm6

        ## Prefetch row after bottom
        prefetch [esi + nxt]
        prefetchw [edi + yPitch]

        ## xLoop finish
        add     edi, 8
        dec     ecx
        jne     xLoop

        dec     ebx
        jne     yLoop

        ## Fix stack
        mov     esp, ebp
        sub     esp, 12

        pop     ebx
        pop     edi
        pop     esi

        pop     ebp

        emms

        ret
