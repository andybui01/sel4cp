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

#include <sel4/sel4.h>

typedef unsigned int sel4cp_channel;
typedef unsigned int sel4cp_pd;
typedef unsigned int sel4cp_thread;
typedef seL4_MessageInfo_t sel4cp_msginfo;

typedef unsigned int sel4cp_errno;

#define SEL4CP_MAX_USER_THREADS (64)
#define SEL4CP_MAX_CHILD_PDS (1)

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
    
    /* SchedControl cap to configure SchedContexts */
    SCHEDCONTROL_CPTR,

    BASE_OUTPUT_NTFN_CPTR = 10,
    BASE_OUTPUT_EP_CPTR = BASE_OUTPUT_NTFN_CPTR + 64,
    BASE_IRQ_CPTR = BASE_OUTPUT_EP_CPTR + 64,
    BASE_TCB_CPTR = BASE_IRQ_CPTR + 64,
    BASE_SC_CPTR = BASE_TCB_CPTR + SEL4CP_MAX_USER_THREADS,
    BASE_CSPACE_CPTR = BASE_SC_CPTR + SEL4CP_MAX_USER_THREADS,
    BASE_VSPACE_CPTR = BASE_CSPACE_CPTR + SEL4CP_MAX_USER_THREADS,
    MAX_CSPACE_CPTR = BASE_VSPACE_CPTR + SEL4CP_MAX_CHILD_PDS,
};

#define SEL4CP_MAX_CHANNELS 63

/* User provided functions */
void init(void);
void notified(sel4cp_channel ch);
sel4cp_msginfo protected(sel4cp_channel ch, sel4cp_thread thread, sel4cp_msginfo msginfo);
void fault(sel4cp_channel ch, sel4cp_thread thread, sel4cp_msginfo msginfo);

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
sel4cp_thread_restart(sel4cp_thread thread, uintptr_t entry_point)
{
    seL4_Error err;
    seL4_UserContext ctxt = {0};
    ctxt.pc = entry_point;
    err = seL4_TCB_WriteRegisters(
        BASE_TCB_CPTR + thread,
        true,
        0, /* No flags */
        1, /* writing 1 register */
        &ctxt
    );

    if (err != seL4_NoError) {
        sel4cp_dbg_puts("sel4cp_thread_restart: error writing registers\n");
        sel4cp_internal_crash(err);
    }
}

static inline void
sel4cp_thread_resume(sel4cp_thread thread)
{
    seL4_TCB_Resume(BASE_TCB_CPTR + thread);
}

static inline void
sel4cp_thread_stop(sel4cp_thread thread)
{
    seL4_Error err;
    err = seL4_TCB_Suspend(BASE_TCB_CPTR + thread);
    if (err != seL4_NoError) {
        sel4cp_dbg_puts("sel4cp_thread_stop: error stopping thread\n");
        sel4cp_internal_crash(err);
    }
}

typedef struct {
    /* ID of protection domain to spawn the thread inside. */
    sel4cp_pd pd;
    uint64_t budget; // = TIME_CAPACITY
    uint64_t period;
    uintptr_t entry_point;

} sel4cp_thread_attr;

/**
 * @brief Spawns a thread inside a proection domain.
 * 
 * @param[in]   pd      The id of the protection domain to spawn the thread inside.
 * @param[out]  thread  Thread ID of the newly spawned thread.
 * @return 0 on success, errno otherwise.
 */
static inline sel4cp_errno
sel4cp_spawn_thread(sel4cp_thread_attr *thread_attr, sel4cp_thread thread)
{
    /** TODO: proper error handling */
    seL4_SchedControl_ConfigureFlags(
        SCHEDCONTROL_CPTR,
        BASE_SC_CPTR + thread,
        thread_attr->budget,
        thread_attr->period,
        0,
        thread, /** FIXME: what should the badge be set to? make sure timeout endpoint is correct */
        0
    );

    // seL4_TCB_Configure(
    //     BASE_TCB_CPTR + thread,
    //     BASE_CSPACE_CPTR + thread,
    //     0,
    //     BASE_VSPACE_CPTR + thread_attr->pd, /** FIXME: pd_ids are 1-indexed */
    //     0,
    //     /* IPC Buffer address */,
    //     /* seL4 Frame for IPC Buffer */
    // );

    seL4_UserContext ctxt = {0};
    ctxt.pc = thread_attr->entry_point;
    seL4_TCB_WriteRegisters(
        BASE_TCB_CPTR + thread,
        false,
        0,
        1,
        &ctxt
    );
    return 0;
}

static inline sel4cp_msginfo
sel4cp_ppcall(sel4cp_channel ch, sel4cp_msginfo msginfo)
{
    return seL4_Call(BASE_OUTPUT_EP_CPTR + ch, msginfo);
}

static inline sel4cp_msginfo
sel4cp_root_ppcall(sel4cp_msginfo msginfo)
{
    return seL4_Call(ROOT_PD_EP_CPTR, msginfo);
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
