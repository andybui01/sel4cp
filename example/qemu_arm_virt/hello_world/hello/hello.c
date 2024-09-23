#include <stdint.h>
#include <sel4cp/sel4cp.h>

#include "../os/util.h"

#define SYSCALL_DUMMY 0
#define SYSCALL_THREAD_CREATE 1
#define SYSCALL_THREAD_BLOCK 2
#define SYSCALL_THREAD_RELEASE 3

void
worker_thread(void)
{
    sel4cp_dbg_puts("THREAD 1: Releasing thread 0\n");
    sel4cp_msginfo msginfo = sel4cp_root_ppcall(sel4cp_msginfo_new(SYSCALL_THREAD_RELEASE, 0));
    seL4_Assert(sel4cp_msginfo_get_label(msginfo) == 0);
}

void
init(void)
{
    extern void sel4cp_thread_entry(sel4cp_thread thread, bool is_root_thread);

    sel4cp_dbg_puts("PARTITION 1: SPAWN NEW THREAD\n");
    sel4cp_mr_set(0, (uintptr_t)sel4cp_thread_entry);
    sel4cp_mr_set(1, (uintptr_t)worker_thread);

    sel4cp_msginfo msginfo = sel4cp_root_ppcall(sel4cp_msginfo_new(SYSCALL_THREAD_CREATE, 2));
    seL4_Assert(sel4cp_msginfo_get_label(msginfo) == 0);

    sel4cp_dbg_puts("THREAD 0: blocking myself\n");
    msginfo = sel4cp_root_ppcall(sel4cp_msginfo_new(SYSCALL_THREAD_BLOCK, 0));
    sel4cp_dbg_puts("THREAD 0: freedom!\n");

    for (;;) {
        sel4cp_dbg_puts("PARTITION 1: IM IN THREAD 1 IM IN THREAD 1 IM IN THREAD 1 IM IN THREAD 1\n");
    }

}

void
notified(sel4cp_channel ch)
{
}