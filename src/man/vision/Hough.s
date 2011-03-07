/* -*- mode: asm; indent-tabs-mode: nil -*- */
.intel_syntax noprefix

.globl _mark_edges

.section .data

        ## Parameter layout
                .struct 8
num_peaks:      .skip 4
angle_spread:   .skip 4
peaks:          .skip 4
hough_space:

        ## Peak list layout
        .struct 0
angle_offset:   .skip 2
x_offset:       .skip 2
y_offset:

# sin(0 + 0x40) = cos(x) (0x40 = 90 degrees, btw)
        .equiv sin_offset,     0x40*2 # needs to offset for words (2 bytes each)

        ## Stack layout
        .struct 0
spread: .skip 4
end_of_stack:

        .equiv  r_span, 400
        .equiv  t_span, 256
        .equiv  bytes_per_bin, 2 # hough space has uint16_t bins

        .data

        ## Calculated in Excel as ROUND( SIN(angle * PI/128) * 2^14)
        ## Angle goes from 0 -> 320 (to make room for sin and cos values)
cosTable:
        .word   16384, 16379, 16364, 16340, 16305, 16261, 16207, 16143
        .word   16069, 15986, 15893, 15791, 15679, 15557, 15426, 15286
        .word   15137, 14978, 14811, 14635, 14449, 14256, 14053, 13842
        .word   13623, 13395, 13160, 12916, 12665, 12406, 12140, 11866
        .word   11585, 11297, 11003, 10702, 10394, 10080, 9760, 9434
        .word   9102, 8765, 8423, 8076, 7723, 7366, 7005, 6639
        .word   6270, 5897, 5520, 5139, 4756, 4370, 3981, 3590
        .word   3196, 2801, 2404, 2006, 1606, 1205, 804, 402
        .word   0, -402, -804, -1205, -1606, -2006, -2404, -2801
        .word   -3196, -3590, -3981, -4370, -4756, -5139, -5520, -5897
        .word   -6270, -6639, -7005, -7366, -7723, -8076, -8423, -8765
        .word   -9102, -9434, -9760, -10080, -10394, -10702, -11003, -11297
        .word   -11585, -11866, -12140, -12406, -12665, -12916, -13160, -13395
        .word   -13623, -13842, -14053, -14256, -14449, -14635, -14811, -14978
        .word   -15137, -15286, -15426, -15557, -15679, -15791, -15893, -15986
        .word   -16069, -16143, -16207, -16261, -16305, -16340, -16364, -16379
        .word   -16384, -16379, -16364, -16340, -16305, -16261, -16207, -16143
        .word   -16069, -15986, -15893, -15791, -15679, -15557, -15426, -15286
        .word   -15137, -14978, -14811, -14635, -14449, -14256, -14053, -13842
        .word   -13623, -13395, -13160, -12916, -12665, -12406, -12140, -11866
        .word   -11585, -11297, -11003, -10702, -10394, -10080, -9760, -9434
        .word   -9102, -8765, -8423, -8076, -7723, -7366, -7005, -6639
        .word   -6270, -5897, -5520, -5139, -4756, -4370, -3981, -3590
        .word   -3196, -2801, -2404, -2006, -1606, -1205, -804, -402
        .word   -0, 402, 804, 1205, 1606, 2006, 2404, 2801
        .word   3196, 3590, 3981, 4370, 4756, 5139, 5520, 5897
        .word   6270, 6639, 7005, 7366, 7723, 8076, 8423, 8765
        .word   9102, 9434, 9760, 10080, 10394, 10702, 11003, 11297
        .word   11585, 11866, 12140, 12406, 12665, 12916, 13160, 13395
        .word   13623, 13842, 14053, 14256, 14449, 14635, 14811, 14978
        .word   15137, 15286, 15426, 15557, 15679, 15791, 15893, 15986
        .word   16069, 16143, 16207, 16261, 16305, 16340, 16364, 16379
        .word   16384, 16379, 16364, 16340, 16305, 16261, 16207, 16143
        .word   16069, 15986, 15893, 15791, 15679, 15557, 15426, 15286
        .word   15137, 14978, 14811, 14635, 14449, 14256, 14053, 13842
        .word   13623, 13395, 13160, 12916, 12665, 12406, 12140, 11866
        .word   11585, 11297, 11003, 10702, 10394, 10080, 9760, 9434
        .word   9102, 8765, 8423, 8076, 7723, 7366, 7005, 6639
        .word   6270, 5897, 5520, 5139, 4756, 4370, 3981, 3590
        .word   3196, 2801, 2404, 2006, 1606, 1205, 804, 402
        .word   0

.section .text

        ## Puts r in \x using following formula
        ## r = x * cos(t) + y * sin(t)
.macro GET_R x, y, t, empty
##########################
        ## Alters \x, \y, and \empty
        ## Does not alter \t
        ## \x, \y, and \t MUST be 32-bit registers
##########################

        ## Load cosine into a 32-bit register and multiply by 32 bit x
        movsx   \empty, word ptr[cosTable + \t*2] # table is 16-bit values
        imul    \x, \empty

        ## Load sine into a 32-bit register and multiply by 32 bit x
	## table vals are 16-bits
        movsx   \empty, word ptr[cosTable + sin_offset + \t*2]
        imul    \y, \empty

        ## Sum x*cos(t) + y*sin(t)
        add     \x, \y
        shr     \x, 14

        ## Set \y to zero
        xor     \y,\y

        ## Put the R value in the Hough Space coordinates
        add     \x, r_span/2

        ## Ensure R >= 0
        ## If \x + r_span/2 < 0 (less than, than set it to zero)
        cmovl   \x, \y

        ## Ensure R < r_span
        mov     \y, r_span - 1
        cmp     \x, \y
        cmova   \x, \y
.endm

        ## _mark_edges(int numPeaks, int angleSpread,
        ##             uint16_t *peaks, int *houghSpace)
_mark_edges:
        push    ebp
        mov     ebp, esp

        push    esi
        push    edi
        push    ebx

        sub     esp, end_of_stack

peakLoop:
        ## Load pointer to peak list
        mov     edi, dword ptr[ebp + peaks]

        ## Load angleSpread parameter
        mov     eax, [ebp + angle_spread]

        ## Get gradient angle lower bound
        movzx   esi, word ptr[edi + angle_offset]

        ## esi = t0 - angleSpread
        sub     esi, eax

        ## Make sure esi is 0 <= t < t_span (angles are only positive)
        and     esi, t_span-1

        ## Double spread param and store it on the stack as a counter
        shl     eax, 1
        mov     dword ptr[esp + spread], eax

        ## Load peak's x and y values
        movsx   ebx, word ptr[edi + x_offset]
        movsx   eax, word ptr[edi + y_offset]

        ## put r0, radius at t-angleSpread in ebx
        GET_R   ebx, eax, esi, ecx

        ## Iterate from lower gradient angle to upper
gradientLoop:
        ## Load pointer to peak list
        mov     edi, dword ptr[ebp + peaks]

        ## edx = x
        ## eax = y
        movsx   edx, word ptr[edi + x_offset]
        movsx   eax, word ptr[edi + y_offset]

        ## Get radius at current angle + 1
        mov     ecx, esi
        add     ecx, 1

        ## ebx = r0, edx = r1
        GET_R   edx, eax, ecx, edi

        ## Calculate hough space address
        mov     eax, esi
        imul    eax, (r_span + 1) * bytes_per_bin # width is r_span + 1, actually
        add     eax, dword ptr[ebp + hough_space]

        ## edi = min(r0,r1), ecx = max(r0, r1)
        cmp     edx, ebx

        ## If edx > ebx
        cmova   ecx, edx
        cmova   edi, ebx

        ## If edx <= ebx
        cmovbe  ecx, ebx
        cmovbe  edi, edx

        ## ecx = maxR - minR
        ## rDiff counter
        sub     ecx, edi

        ## Iterate from minR to maxR
        ## for ( ; rDiff < 0; rDiff++)
radiiLoop:
        # ti * r_span + ri * bytes_per_bin
        inc     word ptr[eax + edi * bytes_per_bin]

        ## radiiLoop
        inc     edi             # Increment ri
        dec     ecx             # Increment rDiff counter
        jge     radiiLoop       # rDiff >= 0, means minR < ri <= maxR

        ## r0 = r1
        mov     ebx, edx

        ## gradientLoop
        inc     esi             # Increment ti
        and     esi, t_span-1   # 0 <= t < t_span

        dec     dword ptr[esp + spread]
        jne     gradientLoop

        ## 6 bytes per x,y,angle triple
        add     dword ptr[ebp + peaks], 6

        dec     dword ptr[ebp + num_peaks]
        jne     peakLoop

        add     esp, end_of_stack

        pop     ebx
        pop     edi
        pop     esi

        mov     esp, ebp
        pop     ebp

        ret

