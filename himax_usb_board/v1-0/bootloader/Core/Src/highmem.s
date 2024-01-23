/**
 * This file is a hack that lets us have all bits in the bootloader's reset vector set.
 * Because the "Reset_Trampoline" sits at the highest address in flash (which, by luck,
 * is 0x0800_fffe for this MCU), we don't need to erase that address before writing it.
 *
 * Therefore, we can leave the startup vector table programmed to jump to the bootloader until
 * we're ready to finalize the code with a single write to the reset vector address.
 */


    .syntax unified
    .cpu cortex-m7
    .fpu softvfp
    .thumb

    .section .highmem.end
    .type Highmem_Trampoline, %function
Highmem_Trampoline:
    ldr r0, =Reset_Handler
    bx r0

.ltorg

    .section .highmem.end
    .weak Highmem_Entry
    .type Highmem_Entry, %function
Highmem_Entry:
    b Highmem_Trampoline
