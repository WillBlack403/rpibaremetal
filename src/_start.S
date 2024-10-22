//Author william black




#include"arm.h"

            .section ".text.boot"

            .global _start
            .func   _start
_start:
            cpsid if, #ARM_MODE_SYS
            //FPU
            //mrc     p15, 0, r0, c1, c0, #2
            //orr     r0, #(0x300000 + 0xC00000)
            //mcr     p15, 0, r0, c1, c0, #2
            //mov     r0, #0x40000000
            //vmsr    fpexc, r0
            //Branch Prediction
            mrc     p15, 0, r0, c1, c0, #0
            orr     r0, #0x4
            orr     r0, #0x800
            orr     r0, #0x1000
            mcr     p15, 0, r0, c1, c0, #0

            ldr     r0, =_isr_table
            mov     r1, #0x0
            ldmia   r0!,{r2-r9}
            stmia   r1!,{r2-r9}
            ldmia   r0!,{r2-r9}
            stmia   r1!,{r2-r9}
            ldr     r0, =__SYS_stack
            mov     sp, r0
            ldr     r0, =__bss_start
            ldr     r1, =__bss_end
            mov     r2, #0
            b       test
clear_bss:  str     r2,[r0],#4
test:       cmp     r0,r1
            blt     clear_bss

            bl       entry



noexit:     wfi
            b noexit
            .endfunc
/*_start:     
            mov     r12, pc                                             //Store program counter
            mrs     r0, CPSR                                            //grab cpu mode
            orr     r0, r0, #(ARM_I_BIT | ARM_F_BIT)                    //Disable IRQs and FIQS
                                                                        //Set up stacks
            ldr     r2, = __SVC_stack                           
            ldr     r3, = __FIQ_stack  
            ldr     r4, = __IRQ_stack
            ldr     r5, = __SYS_stack
            bic     r0, r0, #ARM_MODE_MASK
            orr     r0, r0, #ARM_MODE_SVC
            msr     CPSR_c, r0
            mov     sp, r2
            bic     r0, r0, #ARM_MODE_MASK
            orr     r0, r0, #ARM_MODE_FIQ
            msr     CPSR_c, r0
            mov     sp, r3
            bic     r0, r0, #ARM_MODE_MASK
            orr     r0, r0, #ARM_MODE_IRQ
            msr     CPSR_c, r0
            mov     sp, r4
            bic     r0, r0, #ARM_MODE_MASK
            orr     r0, r0, #ARM_MODE_SYS
            msr     CPSR_c, r0
            mov     sp, r5
                                                                        //Set up fpu
            mrc     p15, 0, r0, c1, c0, #2
            orr     r0, #(0x300000 + 0xC00000)
            mcr     p15, 0, r0, c1, c0, #2
            mov     r0, #0x40000000
            vmsr    fpexc, r0
                                                                        //Brach Prediciton
            mrc     p15, 0, r0, c1, c0, #0
            orr     r0, #0x4
            orr     r0, #0x800
            orr     r0, #0x1000
            mcr     p15, 0, r0, c1, c0, #0


            ldr     r0, =_isr_table
            mov     r1, #0x0
            ldmia   r0!,{r2-r9}
            stmia   r1!,{r2-r9}
            ldmia   r0!,{r2-r9}
            stmia   r1!,{r2-r9}
            ldr     r0, =__bss_start
            ldr     r1, =__bss_end     //Linker Has built things.
            eor     r2, r2, r2         //zero r2
bss_clear:  cmp     r0, r1
            bge     exit
            str     r2, [r0], #4
            sub     r1, r1, #1
            b       bss_clear
            
            ldr     sp, =_start         //Set the stack at top of code, C - will always use the stack correctly.
exit:       b      entry               //What Is main :)
            .endfunc

            .global noexit
            .func   noexit
noexit:     wfi
            b       noexit              //Hang
            .endfunc
*/


            .global enable
            .func   enable
enable:
            cpsie  i
            mov    pc, lr
            .endfunc

            .global disable
            .func   disable
disable:    
            mrs     r0, cpsr
            cpsid   i
            mov     pc, lr
            .endfunc

_isr_table:
    ldr pc, _reset_h
    ldr pc, _undefined_instuction_vector_h
    ldr pc, _software_interrupt_vector_h
    ldr pc, _prefetch_abort_vector_h
    ldr pc, _data_abort_vector_h
    ldr pc, _unused_handler_h
    ldr pc, _interrupt_vector_h
    ldr pc, _fast_interrupt_vector_h

_reset_h:                       .word   _start
_undefined_instuction_vector_h: .word   noexit
_software_interrupt_vector_h:   .word   noexit
_prefetch_abort_vector_h:       .word   noexit
_data_abort_vector_h:           .word   noexit
_unused_handler_h:              .word   noexit
_interrupt_vector_h:            .word   irq_h
_fast_interrupt_vector_h:       .word   noexit


    .global dmp
    .func dmp
dmp:
    mov r12, #0
    mcr p15,0,r12,c7,c5,0 
    mcr p15,0,r12,c7,c5,6
    mcr p15,0,r12,c7,c10,4
    mcr p15,0,r12,c7,c5,4
    mcr p15,0,r12,c7,c10,5
    mov pc,lr
    .endfunc

    .func irq_h
irq_h:
    sub lr, lr, #4
    srsdb #ARM_MODE_SYS!
    cpsid if, #ARM_MODE_SYS
    push {r0-r4, r12,lr}
    and r4, sp, #4
    sub sp, sp, r4

    bl dmp
    bl disbatch
    bl dmp
    add sp, sp, r4
    pop {r0-r4,r12,lr}
    rfeia sp!
    .endfunc 


