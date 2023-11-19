/*
 * Copyright 2021, Breakaway Consulting Pty. Ltd.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <sel4/sel4.h>

#include <sel4cp.h>

/* SYSINIT needs to handle this */
#define UNBIND_SC_LABEL 0

#define EP_MASK_BIT         63
#define FAULT_EP_MASK_BIT   62
#define FAULT_EP_ROOT_BIT   61

#define TID_MASK        0xff
#define BADGE_TID_BIT   8
#define BADGE_TO_TID(x) (((x) >> BADGE_TID_BIT) & TID_MASK)
#define PD_MASK         0xff
#define CHANNEL_MASK    0x3f

bool passive;
char sel4cp_name[16];
bool have_signal = false;
seL4_CPtr signal;
seL4_MessageInfo_t signal_msg;

__attribute__((weak)) sel4cp_msginfo protected(sel4cp_channel ch, sel4cp_thread thread, sel4cp_msginfo msginfo)
{
    return seL4_MessageInfo_new(0, 0, 0, 0);
}

__attribute__((weak)) void fault(sel4cp_pd pd, sel4cp_thread thread, sel4cp_msginfo msginfo)
{
}

extern void init(void);
extern void notified(sel4cp_channel);

static void
handler_loop(void)
{
    bool have_reply = false;
    seL4_MessageInfo_t reply_tag;

    for (;;) {
        seL4_Word badge;
        seL4_MessageInfo_t tag;

        if (have_reply) {
            tag = seL4_ReplyRecv(INPUT_CPTR, reply_tag, &badge, REPLY_CPTR);
        } else if (have_signal) {
            tag = seL4_NBSendRecv(signal, signal_msg, INPUT_CPTR, &badge, REPLY_CPTR);
        } else {
            tag = seL4_Recv(INPUT_CPTR, &badge, REPLY_CPTR);
        }

        uint64_t is_endpoint = badge >> EP_MASK_BIT;
        uint64_t is_fault = (badge >> FAULT_EP_MASK_BIT) & 1;

        have_reply = false;

        if (is_fault) {
            if ((badge >> FAULT_EP_ROOT_BIT) & 1) {
                /* Fault came from root pd, abort for now */
                seL4_Fail("sel4cp rootpd cannot handle its own fault yet");
            }

            fault(badge & PD_MASK, BADGE_TO_TID(badge), tag);
        } else if (is_endpoint) {
            have_reply = true;
            reply_tag = protected(badge & CHANNEL_MASK, BADGE_TO_TID(badge), tag);
        } else {
            unsigned int idx = 0;
            do  {
                if (badge & 1) {
                    notified(idx);
                }
                badge >>= 1;
                idx++;
            } while (badge != 0);
        }
    }
}

int
main(void)
{
    init();

    /* 
     If we are passive, now our initialisation is complete we can
     signal the monitor to unbind our scheduling context and bind
     it to our notification object. 
     We delay this signal so we are ready waiting on a recv() syscall
    */
    if (passive) {
        have_signal = true;
        signal_msg = seL4_MessageInfo_new(UNBIND_SC_LABEL, 0, 0, 1);
        seL4_SetMR(0, 0);
        signal = (SYSINIT_CPTR);
    }

    handler_loop();

    return 0;
}
