#include <libdummycrt/thread.h>
#include <stddef.h>
#include <stdint.h>

extern const void (*const __init_array_start []) (void);
extern const void (*const __init_array_end []) (void);

extern int main(void);

static void run_init_funcs(void)
{
    size_t count = __init_array_end - __init_array_start;
    for (size_t i = 0; i < count; i++) {
        __init_array_start[i]();
    }
}

void crt_start_c(uint64_t tid)
{
    run_init_funcs();
    sel4cp_thread_setup(tid, main);
}
