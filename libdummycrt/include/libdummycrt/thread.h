#pragma once

#define VSPACE_TLS_START        0x400000
#define VSPACE_TLS_END          0x404000
#define VSPACE_IPCBUFF_START    VSPACE_TLS_END
#define VSPACE_IPCBUFF_SIZE     0x1000
#define VSPACE_STACK_SIZE   0x1000
#define VSPACE_STACK_ALIGN  16
#define VSPACE_STACK_HIGH   0x600000

#ifndef __ASSEMBLER__
#include <stdint.h>

void sel4cp_thread_setup(uint64_t tid, int (*entry)(void));
#endif
