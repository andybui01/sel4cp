/*
 * Copyright 2021, Breakaway Consulting Pty. Ltd.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
/* seL4 Core Platform interface */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdbool.h>

#define __thread
#include <sel4/sel4.h>

typedef unsigned int sel4cp_channel;
typedef unsigned int sel4cp_pd;
typedef seL4_MessageInfo_t sel4cp_msginfo;

enum pd_cspace_caps {
    /* Common between PD and its threads */
    ROOT_PD_EP_CPTR = 1, // Used for syscalls

    /* PD caps */
    REPLY_CPTR = 5,
    INPUT_CPTR,

    /* Not used, however, we need to think about how passive PDs are
     * gonna get their SCs unbound in the future. Also need to think
     * about what PDs/threads in our system will be active/passive. */
    SYSINIT_CPTR,

    BASE_OUTPUT_NTFN_CPTR = 10,
    BASE_OUTPUT_EP_CPTR = BASE_OUTPUT_NTFN_CPTR + 64,
    BASE_IRQ_CPTR = BASE_OUTPUT_EP_CPTR + 64,
    BASE_TCB_CPTR = BASE_IRQ_CPTR + 64,
};

#define SEL4CP_MAX_CHANNELS 63

/* User provided functions */
void init(void);
void notified(sel4cp_channel ch);
sel4cp_msginfo protected(sel4cp_channel ch, sel4cp_msginfo msginfo);
void fault(sel4cp_channel ch, sel4cp_msginfo msginfo);

extern char sel4cp_name[16];
/* These next three variables are so our PDs can combine a signal with the next Recv syscall */
extern bool have_signal;
extern seL4_CPtr signal;
extern seL4_MessageInfo_t signal_msg;

/*
 * Output a single character on the debug console.
 */
void sel4cp_dbg_putc(int c);

/*
 * Output a NUL terminated string to the debug console.
 */
void sel4cp_dbg_puts(const char *s);

static inline void
sel4cp_internal_crash(seL4_Error err)
{
    /*
     * Currently crash be dereferencing NULL page
     *
     * Actually derference 'err' which means the crash reporting will have
     * `err` as the fault address. A bit of a cute hack. Not a good long term
     * solution but good for now.
     */
    int *x = (int *)(uintptr_t) err;
    *x = 0;
}

static inline void
sel4cp_notify(sel4cp_channel ch)
{
    seL4_Signal(BASE_OUTPUT_NTFN_CPTR + ch);
}

static inline void
sel4cp_irq_ack(sel4cp_channel ch)
{
    seL4_IRQHandler_Ack(BASE_IRQ_CPTR + ch);
}

static inline void
sel4cp_pd_restart(sel4cp_pd pd, uintptr_t entry_point)
{
    seL4_Error err;
    seL4_UserContext ctxt = {0};
    ctxt.pc = entry_point;
    err = seL4_TCB_WriteRegisters(
        BASE_TCB_CPTR + pd,
        true,
        0, /* No flags */
        1, /* writing 1 register */
        &ctxt
    );

    if (err != seL4_NoError) {
        sel4cp_dbg_puts("sel4cp_pd_restart: error writing registers\n");
        sel4cp_internal_crash(err);
    }
}

static inline void
sel4cp_pd_stop(sel4cp_pd pd)
{
    seL4_Error err;
    err = seL4_TCB_Suspend(BASE_TCB_CPTR + pd);
    if (err != seL4_NoError) {
        sel4cp_dbg_puts("sel4cp_pd_restart: error writing registers\n");
        sel4cp_internal_crash(err);
    }
}

static inline sel4cp_msginfo
sel4cp_ppcall(sel4cp_channel ch, sel4cp_msginfo msginfo)
{
    return seL4_Call(BASE_OUTPUT_EP_CPTR + ch, msginfo);
}

static inline sel4cp_msginfo
sel4cp_msginfo_new(uint64_t label, uint16_t count)
{
    return seL4_MessageInfo_new(label, 0, 0, count);
}

static inline uint64_t
sel4cp_msginfo_get_label(sel4cp_msginfo msginfo)
{
    return seL4_MessageInfo_get_label(msginfo);
}

static void
sel4cp_mr_set(uint8_t mr, uint64_t value)
{
    seL4_SetMR(mr, value);
}

static uint64_t
sel4cp_mr_get(uint8_t mr)
{
    return seL4_GetMR(mr);
}
