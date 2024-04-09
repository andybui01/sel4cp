#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <sel4/sel4.h>
#include <sel4cp/sel4cp.h>

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
    seL4_Error err = seL4_TCB_Resume(BASE_TCB_CPTR + thread);
    seL4_Assert(!err);
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

static sel4cp_errno
sel4cp_thread_set_sched_params(sel4cp_thread thread, uint64_t budget, uint64_t period)
{
    seL4_Error err;
    err = seL4_SchedControl_ConfigureFlags(
        SCHEDCONTROL_CPTR,
        BASE_SC_CPTR + thread,
        budget,
        period,
        0,
        0, /** FIXME: what should the badge be set to? for now we don't have timeout endpoints */
        0
    );
    seL4_Assert(!err);

    return err;
}

static sel4cp_errno
sel4cp_thread_set_address_space(sel4cp_thread thread, sel4cp_pd pd)
{
    seL4_Error err;
    err = seL4_TCB_SetVSpace(
        BASE_TCB_CPTR + thread,
        BASE_VSPACE_CPTR + pd,
        0
    );
    seL4_Assert(!err);
    
    return err;
}

static sel4cp_errno
sel4cp_thread_set_priority(sel4cp_thread thread, sel4cp_pd authority, uint64_t priority)
{
    seL4_Error err;

    /* Use the PD's MCP */
    err = seL4_TCB_SetPriority(BASE_TCB_CPTR + thread, BASE_TCB_CPTR + authority, priority);
    seL4_Assert(!err);

    return err;
}

static sel4cp_errno
sel4cp_thread_set_entry_attr(sel4cp_thread thread, uintptr_t thread_entry, uintptr_t real_entry,
                             uintptr_t sp, uintptr_t ipc_buffer, uintptr_t tls_memory)
{
    seL4_Error err;
    seL4_UserContext ctxt;

    ctxt.pc = thread_entry;
    ctxt.sp = sp;
    ctxt.spsr = 0; // unused, but I'm being verbose so nregisters make sense
    ctxt.x0 = ipc_buffer;
    ctxt.x1 = tls_memory;
    ctxt.x2 = real_entry;
    size_t nregisters = 6;

    err = seL4_TCB_WriteRegisters(
        BASE_TCB_CPTR + thread,
        false,
        0,
        nregisters,
        &ctxt
    );
    seL4_Assert(!err);

    return err;
}
