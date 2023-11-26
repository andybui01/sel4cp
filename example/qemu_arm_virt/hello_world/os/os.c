#include <stdint.h>
#include <sel4cp.h>

void
init(void)
{
}

sel4cp_msginfo protected(sel4cp_channel ch, sel4cp_thread thread, sel4cp_msginfo msginfo)
{
    seL4_Assert(thread == 0);
    seL4_Assert(sel4cp_msginfo_get_label(msginfo) == 0xdeadbeef);
    seL4_Assert(sel4cp_mr_get(0) == 0xdead);
    sel4cp_dbg_puts("os received call! replying...\n");
    return sel4cp_msginfo_new(0xd00d, 0);
}

void
notified(sel4cp_channel ch)
{
}