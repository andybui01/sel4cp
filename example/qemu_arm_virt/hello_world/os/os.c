#include <stdint.h>
#include <sel4cp.h>
#include <stddef.h>

#define SYSCALL_THREAD_CREATE 1

/** TODO: We have a bit of a problem, we need to know these values, but they're only determined
 * after the tool runs. So we need a pre-tool step that determines these constants that the PDs
 * can use. This should reduce a lot of the hard coding.
 */
#define VSPACE_STACK_SIZE   0x1000
#define VSPACE_STACK_ALIGN  16
#define VSPACE_STACK_HIGH   0x600000
#define VSPACE_STACK_LOW    (VSPACE_STACK_HIGH - SEL4CP_MAX_USER_THREADS * 2 * VSPACE_STACK_SIZE)

#define VSPACE_TLS_START    0x400000
#define VSPACE_TLS_END      0x404000

#define VSPACE_IPCBUFF_SIZE 0x1000
#define VSPACE_IPCBUFF_LOW  VSPACE_TLS_END
#define VSPACE_IPCBUFF_HIGH (VSPACE_IPCBUFF_LOW + SEL4CP_MAX_USER_THREADS * VSPACE_IPCBUFF_SIZE)

static uintptr_t
__get_thread_stack_region(sel4cp_thread thread)
{
    return VSPACE_STACK_LOW + (thread * VSPACE_STACK_SIZE * 2) + VSPACE_STACK_SIZE;
}

/* We need to know the TLS header memsize and alignment of EACH PD.
 * TODO: Find a way to automatically determine this, because this hardcoding is ehhhhh... 
 * Idea: create a array of size N=#PDs, and let the tool write TLS info into this array.
 *       But this is messy... */
#define PD_HELLO_TLS_MEMSIZE    0x8 // readelf -l tmp_build/hello/hello.elf
#define PD_HELLO_TLS_ALIGN      0x8
#define ROUND_UP(n, b) (((((n) - 1ul) >> (b)) + 1ul) << (b))
static uintptr_t
__get_thread_tls(sel4cp_thread thread)
{
    size_t aligned_size = ROUND_UP(PD_HELLO_TLS_MEMSIZE, PD_HELLO_TLS_ALIGN);
    return VSPACE_TLS_START + aligned_size * thread;
}

static uintptr_t
__get_thread_ipc_buffer(sel4cp_thread thread)
{
    return VSPACE_IPCBUFF_LOW + VSPACE_IPCBUFF_SIZE * thread;
}

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

        uintptr_t sp = __get_thread_stack_region(new_thread) + VSPACE_STACK_SIZE - VSPACE_STACK_ALIGN;
        uintptr_t ipc_buffer = __get_thread_ipc_buffer(new_thread);
        uintptr_t tls_memory = __get_thread_tls(new_thread);
        uintptr_t thread_entry = (uintptr_t)sel4cp_mr_get(0);
        uintptr_t real_entry = (uintptr_t)sel4cp_mr_get(1);

        sel4cp_thread_set_entry_attr(new_thread, thread_entry, real_entry, sp, ipc_buffer, tls_memory);

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