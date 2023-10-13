/*
 * Copyright 2021, Breakaway Consulting Pty. Ltd.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <stdint.h>
#include <sel4cp.h>

/* CSpace, VSpace, etc. caps go here. */
/* @andyb: this needs to be auto-generated from sysxml or something 
 *         and for some reason it needs to be #threads + 1?*/
#define EMPTY_THREADS 67
seL4_Word thread_tcbs[EMPTY_THREADS];
seL4_Word thread_sc[EMPTY_THREADS];

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