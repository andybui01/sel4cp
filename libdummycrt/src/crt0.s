.extern __start_c

.section ".text.start"

.global __start;
.type __start, %function;
__start:
    mov fp, #0
    mov lr, #0

    // x1 contains the IPC Buffer addr, provided by sel4cp already
    // x2 contains a memory area for TLS, also provided by sel4cp
    b __start_c

1:
    b 1b
