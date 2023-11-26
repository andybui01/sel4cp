#include <stdint.h>
#include <sel4cp.h>

void
init(void)
{
    while (1) {
        sel4cp_dbg_puts("hello.c sending pp to the os\n");
        sel4cp_mr_set(0, 0xdead);
        sel4cp_msginfo msginfo = sel4cp_ppcall(0, sel4cp_msginfo_new(0xdeadbeef, 1));
        seL4_Assert(sel4cp_msginfo_get_label(msginfo) == 0xd00d);
    }
}

void
notified(sel4cp_channel ch)
{
}