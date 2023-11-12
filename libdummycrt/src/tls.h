#pragma once

#ifndef __aarch64__
#error TLS only implemented for aarch64.
#endif

#include <stdint.h>

struct tls_info {
    /* Location of the TLS template in memory. */
    void *tls_template;
    /* Total size of the TLS template, .tdata and .tbss. */
    size_t tls_template_size;
    /* Size of the .tdata section. */
    size_t tls_tdata_size;
    /* Alignment of TLS template. */
    size_t align;
};

/**
 * @brief Copy the TLS template into a new thread.
 * 
 * @param[in] tls_env   Info about the TLS, essentially a truncated ELF Phdr.
 * @param[in] memory    Allocated memory region for TLS, may not be aligned.
 * 
 * @return 0 if memory invalid, aligned tp if success.
 */
uintptr_t copy_tls(const struct tls_info *tls_env, void *memory);

/**
 * @brief Set the tp in the arch TLS register.
 * 
 * @param[in] tp    Address of tp in memory to be loaded into TLS register.
 */
static inline void arch_set_tp(const uintptr_t tp)
{
    asm volatile("msr tpidr_el0, %0" :: "r"(tp));
}
