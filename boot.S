.section .multiboot
.align 8
multiboot_header:
    .long 0xe85250d6
    .long 0
    .long multiboot_header_end - multiboot_header
    .long -(0xe85250d6 + 0 + (multiboot_header_end - multiboot_header))

    /* Framebuffer request tag: type, flags, size, width, height, depth (20 bytes total) */
    .short 5
    .short 0
    .long 20
    .long 1024
    .long 768
    .long 32
    .align 8

    /* End tag */
    .short 0
    .short 0
    .long 8
multiboot_header_end:

.section .text
.global _start
.type _start, @function
_start:
    mov $stack_top, %esp
    push %ebx
    push %eax
    call kernel_main
halt:
    cli
    hlt
    jmp halt

.section .bss
.align 16
stack_bottom:
    .skip 16384
stack_top:
