#include <libdummycrt/string.h>
#include <stdint.h>

#include "tls.h"

#define MIN_TCB_SIZE (16)

#define ROUND_UP(n, b) (((((n) - 1ul) >> (b)) + 1ul) << (b))
#define MAX(a,b) (((a)>(b))?(a):(b))

/* Align the tp up as the first object pointed to by the tp is the tcb,
 * which needs to be aligned according to the PT_TLS segment's
 * p_align value. */
static const uintptr_t ensure_tp_aligned(const uintptr_t tp, const size_t p_align)
{
    return ROUND_UP(tp, p_align);
}

/* Get pointer to new TLS image of thread from tp. */
static void *tls_image_ptr_from_tp(const uintptr_t tp, const size_t p_align)
{
    return (void *)(tp + MAX(MIN_TCB_SIZE, p_align));
}

/* As a new thread, copy the TLS template in the ELF file into our memory. */
static void copy_tls_template(const struct tls_info *tls_env, const uintptr_t tp)
{
    unsigned char *tls = tls_image_ptr_from_tp(tp, tls_env->align);

    memcpy(tls, tls_env->tls_template, tls_env->tls_tdata_size);
    void *tbss = &tls[tls_env->tls_tdata_size];
    memset(tbss, 0, tls_env->tls_template_size - tls_env->tls_tdata_size);

}

uintptr_t copy_tls(const struct tls_info *tls_env, const uintptr_t memory)
{
    if (!memory) {
        return 0;
    }

    /* We are given a free block of memory for the TLS, make sure
     * this is aligned. */
    uintptr_t tp = ensure_tp_aligned((const uintptr_t)memory, tls_env->align);

    copy_tls_template(tls_env, tp);

    return tp;
}
