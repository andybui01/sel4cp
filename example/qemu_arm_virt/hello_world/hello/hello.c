#include <stdint.h>
#include <sel4cp/sel4cp.h>

#define SYSCALL_THREAD_CREATE 1

void
worker_thread(void)
{
    // seL4_Assert(0);
    for (;;) {
        sel4cp_dbg_puts("PARTITION 1: IM IN THREAD 2 IM IN THREAD 2 IM IN THREAD 2 IM IN THREAD 2\n");
    }
}

void
init(void)
{
    extern void sel4cp_thread_entry(void *ipc_buffer, void *tls_memory, int (*entry)(void));

    sel4cp_mr_set(0, (uintptr_t)sel4cp_thread_entry);
    sel4cp_mr_set(1, (uintptr_t)worker_thread);
    sel4cp_msginfo msginfo = sel4cp_root_ppcall(sel4cp_msginfo_new(SYSCALL_THREAD_CREATE, 2));
    seL4_Assert(sel4cp_msginfo_get_label(msginfo) == 0);
    seL4_DebugDumpScheduler();

    for (;;) {
        sel4cp_dbg_puts("PARTITION 1: IM IN THREAD 1 IM IN THREAD 1 IM IN THREAD 1 IM IN THREAD 1\n");
    }
}

void
notified(sel4cp_channel ch)
{
}