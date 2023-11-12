=== libstupidcrt ===

An attempt at a dummy C runtime for seL4CP.

Objectives:
- To provide an alternative to the event-based model of seL4CP
- Support threading
- Create a minimal TLS, just for aarch64.
- Be completely backwards compatible with the sel4cp tool. The tool should
  not need to know what it is loading. Just needs to be an ELF file.

TLS:
Every thread is entered via sel4cp_thread_entry(), which does some
initialization with the TLS for the thread before jumping to the
main entry point of the thread.

Each ELF will have a (read-only) TLS segment described by the following
ELF Phdr struct. The TLS segment is read-only as the initialized values
are meant to be copied to each thread.

typedef struct {
    Elf64_Word   p_type;    // PT_TLS
    Elf64_Word   p_flags;   // Unused
    Elf64_Off    p_offset;  // Unused
    Elf64_Addr   p_vaddr;   // Start address of .tdata
    Elf64_Addr   p_paddr;   // Unused
    Elf64_Xword  p_filesz;  // Size of .tdata
    Elf64_Xword  p_memsz;   // Size of .tdata + .tbss (size of template)
    Elf64_Xword  p_align;   // Alignment of TLS template
} Elf64_Phdr;

Terminology:
- TLS image refers to the copies of the .tdata section.
- TLS region is both TLS image + memory.

The TLS initialization is as follows:
1. For each new thread, allocate a TLS region of memory (done by sel4cp already)
2. This could be our tp, but the tp needs to be aligned so we calculate the alignment.
3. From the tp, we offset a certain amount to get to the start of the TLS image.
4. Copy the master TLS image (.tdata) into the thread's TLS image.
5. Zero out the new thread's TLS .tbss.
6. Set the arch TLS base register (tpidr_el0 on aarch64) with the tp.
