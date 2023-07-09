/*
 * Copyright 2021, Breakaway Consulting Pty. Ltd.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
/*
 * The seL4 Core Platform Monitor.
 *
 * The monitor is the initial task in a core platform system.
 *
 * The monitor fulfills two purposes:
 *
 *   1. creating the initial state of the system.
 *   2. acting as the fault handler for for protection domains.
 *
 * Initialisation is performed by executing a number of kernel
 * invocations to create and configure kernel objects.
 *
 * The specific invocations to make are configured by the build
 * tool; the monitor simply reads a data structure to execute
 * each invocation one at a time.
 *
 * The process occurs in a two step manner. The first bootstrap
 * step execute the `bootstrap_invocations` only. The purpose
 * of this bootstrap is to get the system to the point for the
 * `system_invocations` is mapped into the monitors address space.
 * Once this occurs it is possible for the monitor to switch to
 * executing invocation from this second data structure.
 *
 * The motivation for this design is to keep both the initial
 * task image and the initial CNode as small, fixed size entities.
 *
 * Fixed size allows both kernel and monitor to avoid unnecesary
 * recompilation for different system configurations. Keeping things
 * small optimizes overall memory usage.
 *
 *
 */

/*
 * Why this you may ask? Well, the seL4 headers depend on
 * a global `__sel4_ipc_buffer` which is a pointer to the
 * thread's IPC buffer. Which is reasonable enough, passing
 * that explicitly to every function would be annoying.
 *
 * The seL4 headers make this global a thread-local global,
 * which is also reasonable, considering it applies to a
 * specific thread! But, for our purposes we don't have threads!
 *
 * Thread local storage is painful and annoying to configure.
 * We'd really rather NOT use thread local storage (especially
 * consider we never have more than one thread in a Vspace)
 *
 * So, by defining __thread to be empty it means the variable
 * becomes a true global rather than thread local storage
 * variable, which means, we don't need to waste a bunch
 * of effort and complexity on thread local storage implementation.
 */
#define __thread

#include <stdbool.h>
#include <stdint.h>
#include <sel4/sel4.h>

#include "util.h"
#include "debug.h"

#define MAX_PDS 64
#define MAX_NAME_LEN 16
#define MAX_TCBS 64

#define MAX_UNTYPED_REGIONS 256

/* Max words available for bootstrap invocations.
 *
 * Only a small number of syscalls is required to
 * get to the point where the main syscalls data
 * is mapped in, so we keep this small.
 *
 * FIXME: This can be smaller once compression is enabled.
 */
#define BOOTSTRAP_INVOCATION_DATA_SIZE 110

seL4_IPCBuffer *__sel4_ipc_buffer;

char _stack[4096];

// static char pd_names[MAX_PDS][MAX_NAME_LEN];

// seL4_CPtr fault_ep;
// seL4_CPtr reply;
// seL4_CPtr tcbs[MAX_TCBS];
// seL4_CPtr scheduling_contexts[MAX_TCBS];
// seL4_CPtr notification_caps[MAX_TCBS];

struct region {
    uintptr_t paddr;
    uintptr_t size_bits;
    uintptr_t is_device; /*FIXME: should back size_bits / is_device */
};

struct untyped_info {
    seL4_Word cap_start;
    seL4_Word cap_end;
    struct region regions[MAX_UNTYPED_REGIONS];
};

seL4_Word bootstrap_invocation_count;
seL4_Word bootstrap_invocation_data[BOOTSTRAP_INVOCATION_DATA_SIZE];

seL4_Word system_invocation_count;
seL4_Word *system_invocation_data = (void*)0x80000000;

struct untyped_info untyped_info;

void __attribute__((weak))
__assert_fail(const char  *str, const char *file, int line, const char *function)
{
    puts("assert failed: ");
    puts(str);
    puts(" ");
    puts(file);
    puts(" ");
    puts(function);
    puts("\n");
}

static void
check_untypeds_match(seL4_BootInfo *bi)
{
    /* Check that untypeds list generate from build matches the kernel */
    if (untyped_info.cap_start != bi->untyped.start) {
        puts("SYSINIT|ERROR: cap start mismatch. Expected cap start: ");
        puthex32(untyped_info.cap_start);
        puts("  boot info cap start: ");
        puthex32(bi->untyped.start);
        puts("\n");
        fail("cap start mismatch");
    }

    if (untyped_info.cap_end != bi->untyped.end) {
        puts("SYSINIT|ERROR: cap end mismatch. Expected cap end: ");
        puthex32(untyped_info.cap_end);
        puts("  boot info cap end: ");
        puthex32(bi->untyped.end);
        puts("\n");
        fail("cap end mismatch");
    }

    for (unsigned i = 0; i < untyped_info.cap_end - untyped_info.cap_start; i++) {
        if (untyped_info.regions[i].paddr != bi->untypedList[i].paddr) {
            puts("SYSINIT|ERROR: paddr mismatch for untyped region: ");
            puthex32(i);
            puts("  expected paddr: ");
            puthex64(untyped_info.regions[i].paddr);
            puts("  boot info paddr: ");
            puthex64(bi->untypedList[i].paddr);
            puts("\n");
            fail("paddr mismatch");
        }
        if (untyped_info.regions[i].size_bits != bi->untypedList[i].sizeBits) {
            puts("SYSINIT|ERROR: size_bits mismatch for untyped region: ");
            puthex32(i);
            puts("  expected size_bits: ");
            puthex32(untyped_info.regions[i].size_bits);
            puts("  boot info size_bits: ");
            puthex32(bi->untypedList[i].sizeBits);
            puts("\n");
            fail("size_bits mismatch");
        }
        if (untyped_info.regions[i].is_device != bi->untypedList[i].isDevice) {
            puts("SYSINIT|ERROR: is_device mismatch for untyped region: ");
            puthex32(i);
            puts("  expected is_device: ");
            puthex32(untyped_info.regions[i].is_device);
            puts("  boot info is_device: ");
            puthex32(bi->untypedList[i].isDevice);
            puts("\n");
            fail("is_device mismatch");
        }
    }

    puts("SYSINIT|INFO: bootinfo untyped list matches expected list\n");
}

static unsigned
perform_invocation(seL4_Word *invocation_data, unsigned offset, unsigned idx)
{
    seL4_MessageInfo_t tag, out_tag;
    seL4_Error result;
    seL4_Word mr0;
    seL4_Word mr1;
    seL4_Word mr2;
    seL4_Word mr3;
    seL4_Word service;
    seL4_Word service_incr;
    seL4_Word cmd = invocation_data[offset];
    seL4_Word iterations = (cmd >> 32) + 1;
    seL4_Word tag0 = cmd & 0xffffffffULL;
    unsigned int cap_offset, cap_incr_offset, cap_count;
    unsigned int mr_offset, mr_incr_offset, mr_count;
    unsigned int next_offset;

    tag.words[0] = tag0;
    service = invocation_data[offset + 1];
    cap_count = seL4_MessageInfo_get_extraCaps(tag);
    mr_count = seL4_MessageInfo_get_length(tag);

#if 0
        puts("Doing invocation: ");
        puthex32(idx);
        puts(" cap count: ");
        puthex32(cap_count);
        puts(" MR count: ");
        puthex32(mr_count);
        puts("\n");
#endif

    cap_offset = offset + 2;
    mr_offset = cap_offset + cap_count;
    if (iterations > 1) {
        service_incr = invocation_data[mr_offset + mr_count];
        cap_incr_offset = mr_offset + mr_count + 1;
        mr_incr_offset = cap_incr_offset + cap_count;
        next_offset = mr_incr_offset + mr_count;
    } else {
        next_offset = mr_offset + mr_count;
    }

    if (seL4_MessageInfo_get_capsUnwrapped(tag) != 0) {
        fail("kernel invocation should never have unwrapped caps");
    }

    for (unsigned i = 0; i < iterations; i++) {
#if 0
        puts("Preparing invocation:\n");
#endif
        /* Set all the caps */
        seL4_Word call_service = service;
        if (i > 0) {
            call_service += service_incr * i;
        }
        for (unsigned j = 0; j < cap_count; j++) {
            seL4_Word cap = invocation_data[cap_offset + j];
            if (i > 0) {
                cap += invocation_data[cap_incr_offset + j] * i;
            }
#if 0
            puts("   SetCap: ");
            puthex32(j);
            puts(" ");
            puthex64(cap);
            puts("\n");
#endif
            seL4_SetCap(j, cap);
        }

        for (unsigned j = 0; j < mr_count; j++) {
            seL4_Word mr = invocation_data[mr_offset + j];
            if (i > 0) {
                mr += invocation_data[mr_incr_offset + j] * i;
            }
#if 0
            puts("   SetMR: ");
            puthex32(j);
            puts(" ");
            puthex64(mr);
            puts("\n");
#endif
            switch (j) {
                case 0: mr0 = mr; break;
                case 1: mr1 = mr; break;
                case 2: mr2 = mr; break;
                case 3: mr3 = mr; break;
                default: seL4_SetMR(j, mr); break;
            }
        }

        out_tag = seL4_CallWithMRs(call_service, tag, &mr0, &mr1, &mr2, &mr3);
        result = (seL4_Error) seL4_MessageInfo_get_label(out_tag);
        if (result != seL4_NoError) {
            puts("ERROR: ");
            puthex64(result);
            puts(" ");
            puts(sel4_strerror(result));
            puts("  invocation idx: ");
            puthex32(idx);
            puts(".");
            puthex32(i);
            puts("\n");
            fail("invocation error");
        }
#if 0
        puts("Done invocation: ");
        puthex32(idx);
        puts(".");
        puthex32(i);
        puts("\n");
#endif
    }
    return next_offset;
}

void
main(seL4_BootInfo *bi)
{
    __sel4_ipc_buffer = bi->ipcBuffer;
    puts("SYSINIT|INFO: seL4 Core Platform Bootstrap\n");

#if 0
    /* This can be useful to enable during new platform bring up
     * if there are problems
     */
    dump_bootinfo(bi);
#endif

    check_untypeds_match(bi);

    puts("SYSINIT|INFO: Number of bootstrap invocations: ");
    puthex32(bootstrap_invocation_count);
    puts("\n");

    puts("SYSINIT|INFO: Number of system invocations:    ");
    puthex32(system_invocation_count);
    puts("\n");

    unsigned offset = 0;
    for (unsigned idx = 0; idx < bootstrap_invocation_count; idx++) {
        offset = perform_invocation(bootstrap_invocation_data, offset, idx);
    }
    puts("SYSINIT|INFO: completed bootstrap invocations\n");

    offset = 0;
    for (unsigned idx = 0; idx < system_invocation_count; idx++) {
        offset = perform_invocation(system_invocation_data, offset, idx);
    }

    puts("SYSINIT|INFO: completed system invocations\n");

    /* UNREACHABLE: sysinit TCB should be suspended */
    seL4_Fail("Unreachable, sysinit did not self-destruct");
}
