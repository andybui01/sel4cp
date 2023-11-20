#include <stdint.h>
#include <sel4cp.h>

void hm_func(void)
{
    // seL4_Wait(blah, blah);
    return;
}

void
init(void)
{
    /* Init state */
    /* Start fault handler (Health Monitor) */
}

void
notified(sel4cp_channel ch)
{
}