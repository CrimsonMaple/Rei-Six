.section .text.start
.align 4
.global _start
_start:
    mov r11, r0     @ argc
    mov r12, r1     @ argv
    mov r8,  r2     @ magic

    @ Change the stack pointer
    mov sp, #0x08100000
    
    @ Disable caches / MPU
    mrc p15, 0, r4, c1, c0, 0  @ read control register
    bic r4, #(1<<12)           @ - instruction cache disable
    bic r4, #(1<<2)            @ - data cache disable
    bic r4, #(1<<0)            @ - mpu disable
    mcr p15, 0, r4, c1, c0, 0  @ write control register

    @ Invalidate both caches, discarding any data they may contain,
    @ then drain the write buffer
    mov r4, #0
    mcr p15, 0, r4, c7, c5, 0
    mcr p15, 0, r4, c7, c6, 0
    mcr p15, 0, r4, c7, c10, 4

    @ Give read/write access to all the memory regions
    ldr r0, =0x33333333
    mcr p15, 0, r0, c5, c0, 2 @ write data access
    mcr p15, 0, r0, c5, c0, 3 @ write instruction access

    @ Sets MPU permissions and cache settings
    ldr r0, =0xFFFF001D @ ffff0000 32k  | bootrom (unprotected part)
    ldr r1, =0x01FF801D @ 01ff8000 32k  | itcm
    ldr r2, =0x08000029 @ 08000000 2M   | arm9 mem (O3DS / N3DS) 
    ldr r3, =0x10000029 @ 10000000 2M   | io mem (ARM9 / first 2MB)
    ldr r4, =0x20000037 @ 20000000 256M | fcram (O3DS / N3DS)
    ldr r5, =0x1FF00027 @ 1FF00000 1M   | dsp / axi wram
    ldr r6, =0x1800002D @ 18000000 8M   | vram (+ 2MB)
    mov r7, #0
    mov r10, #0x15
    mcr p15, 0, r0, c6, c0, 0
    mcr p15, 0, r1, c6, c1, 0
    mcr p15, 0, r2, c6, c2, 0
    mcr p15, 0, r3, c6, c3, 0
    mcr p15, 0, r4, c6, c4, 0
    mcr p15, 0, r5, c6, c5, 0
    mcr p15, 0, r6, c6, c6, 0
    mcr p15, 0, r7, c6, c7, 0
    mcr p15, 0, r10, c3, c0, 0	@ Write bufferable 0, 2, 4
    mcr p15, 0, r10, c2, c0, 0	@ Data cacheable 0, 2, 4
    mcr p15, 0, r10, c2, c0, 1	@ Inst cacheable 0, 2, 4

    @ Enable caches
    mrc p15, 0, r0, c1, c0, 0  @ read control register
    orr r0, r0, #(1<<18)       @ - ITCM enable
    orr r0, r0, #(1<<13)       @ - alternate exception vectors enable
    orr r0, r0, #(1<<12)       @ - instruction cache enable
    orr r0, r0, #(1<<2)        @ - data cache enable
    orr r0, r0, #(1<<0)        @ - mpu enable
    mcr p15, 0, r0, c1, c0, 0  @ write control register

    @ Fixes mounting of SDMC
    ldr r0, =0x10000020
    mov r1, #0x340
    str r1, [r0]
    
    @ Restore argc, argv, and magic.
    mov r0, r11
    mov r1, r12
    mov r2, r8

    @ Patch and run CFW
    b main

.die:
    b .die