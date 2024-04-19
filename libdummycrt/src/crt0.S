#include <libdummycrt/thread.h>

.extern crt_start_c
.extern sel4cp_thread_setup

.section ".text"
.type setup_stack, %function;
setup_stack:
    /* DO NOT CLOBBER: x0,x1 */

    mov x9, #VSPACE_STACK_HIGH
    mov x10, x0

    mov x11, #VSPACE_STACK_SIZE
    mul x10, x10, x11

    mov x11, #2
    mul x10, x10, x11

    sub x9, x9, x10
    sub x9, x9, #VSPACE_STACK_ALIGN
    mov sp, x9

    ret
.size setup_stack, .-setup_stack

.section ".text.start"
.global sel4cp_thread_entry;
.type sel4cp_thread_entry, %function;
sel4cp_thread_entry:
    /* x0: TID of this PD */
    /* x1: Is this root thread of PD */

    /* NOTE: at this point in time, sp is empty and we do NOT have a stack to use. 
             This assembly function MUST setup the stack before jumping to C code. */
    bl setup_stack

    mov fp, #0
    mov lr, #0

    /* Jump to C code */
    cmp x1, #1
    beq root_thread
    b spawned_thread

1:
    b 1b

root_thread:
    b crt_start_c
    b 1b

spawned_thread:
    b sel4cp_thread_setup
    b 1b
.size sel4cp_thread_entry, .-sel4cp_thread_entry
