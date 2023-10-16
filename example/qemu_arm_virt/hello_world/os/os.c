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

/* How many child PDs will this root PD support? If this number is too
 * small sel4cp will complain :) */
#define MAX_CHILD_PDS 2
seL4_Word thread_tcbs[EMPTY_THREADS];
seL4_Word thread_sc[EMPTY_THREADS];
seL4_Word pd_vspace[MAX_CHILD_PDS];

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