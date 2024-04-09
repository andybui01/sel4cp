#include <stdint.h>
#include <sel4cp/sel4cp.h>

void
init(void)
{
    for (;;) {
        sel4cp_dbg_puts("PARTITION 2: world world world world\n");
        // seL4_DebugDumpScheduler();
    }
}

void
notified(sel4cp_channel ch)
{
}