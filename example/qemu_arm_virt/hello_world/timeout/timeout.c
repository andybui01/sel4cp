#include <stdint.h>
#include <sel4cp/sel4cp.h>

void
init(void)
{
    for (;;) {
        sel4cp_dbg_puts("PARTITION 3: try to timeout\n");
        /* uncomment the line below to make this PD not timeout! */
        // seL4_Yield();
    }
}

void
notified(sel4cp_channel ch)
{
}