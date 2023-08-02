/*
 * Copyright 2021, Breakaway Consulting Pty. Ltd.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <stdint.h>
#include <sel4cp.h>

void
init(void)
{
    while(1) {
        sel4cp_dbg_puts("hello\n");
    }
}

void
notified(sel4cp_channel ch)
{
}