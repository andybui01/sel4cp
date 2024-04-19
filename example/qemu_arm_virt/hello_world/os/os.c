#include <stdint.h>
#include <stddef.h>

#include <sel4cp/sel4cp.h>
#include <sel4cp/thread.h>

#define SYSCALL_THREAD_CREATE 1

void
init(void)
{
}

sel4cp_msginfo protected(bool is_child, sel4cp_identifier identifier, sel4cp_msginfo msginfo)
{
    seL4_Assert(is_child);

    sel4cp_thread thread = (sel4cp_thread)identifier;
    seL4_Assert(thread == 0);

    uint64_t syscall = sel4cp_msginfo_get_label(msginfo);
    switch (syscall) {
    case SYSCALL_THREAD_CREATE:
        sel4cp_thread new_thread = 1; // only PD is id 0, so the thread is 1

        uint64_t budget = 1000;
        uint64_t period = 1000;

        sel4cp_thread_set_sched_params(new_thread, budget, period);

        /* In the beginning, only threads that run are the initial PDs themselves,
         * and so PD ID == thread ID for the initial PDs unless we stop their
         * threads of execution, in which case their ID becomes free.
         * 
         * This is the OS' job to manage. */
        sel4cp_pd pd = thread;

        sel4cp_thread_set_address_space(new_thread, pd);

        uint64_t priority = 200;

        sel4cp_thread_set_priority(thread, pd, priority);

        uintptr_t thread_entry = (uintptr_t)sel4cp_mr_get(0);
        uintptr_t real_entry = (uintptr_t)sel4cp_mr_get(1);

        sel4cp_thread_set_entry_attr(new_thread, thread_entry, real_entry);

        sel4cp_thread_resume(new_thread);
        break;
    default:
        break;
    }

    return sel4cp_msginfo_new(0, 0);
}

void
notified(sel4cp_channel ch)
{
}