from enum import IntEnum
from typing import Tuple

from sel4coreplat.util import kb, round_up

class SystemConfig(IntEnum):
    # Max 1MiB ELF
    MAX_ELF_SIZE        = 0x100000
    MAX_USER_THREADS    = 64

class ProtectionDomainVSpace(IntEnum):
    """
    Layout of PD VSpaces. Everything is grouped in 2MiB blocks to reduce the
    amount of page tables required for each VSpace.
    Note: these constants might not be in order...
    """
    # Where the ELF is loaded - 2MiB for now, this is hardcoded
    # in the ELF linker scripts.
    VSPACE_ELF_LOAD     = 0x200000
    VSPACE_ELF_END      = VSPACE_ELF_LOAD + SystemConfig.MAX_ELF_SIZE

    # TLS region starting from 4MiB, we reserve 16KiB
    VSPACE_TLS_START    = 0x400000
    VSPACE_TLS_END      = 0x404000

    # IPC Buffers - one page for each thread
    VSPACE_IPCBUFF_SIZE = kb(4)
    VSPACE_IPCBUFF_LOW  = VSPACE_TLS_END
    VSPACE_IPCBUFF_HIGH = VSPACE_IPCBUFF_LOW + SystemConfig.MAX_USER_THREADS * VSPACE_IPCBUFF_SIZE

    # Between each PD's stack, we add a guard page to catch stack overflows.
    # NOTE: an additional page above STACK_HIGH is reserved as a guard page.
    VSPACE_STACK_SIZE   = kb(4)
    VSPACE_STACK_ALIGN  = 16
    VSPACE_STACK_HIGH   = 0x600000
    VSPACE_STACK_LOW    = VSPACE_STACK_HIGH - SystemConfig.MAX_USER_THREADS * 2 * VSPACE_STACK_SIZE

    @classmethod
    def alloc_thread_stack(cls, tid: int) -> Tuple[int, int]:
        """
        Allocate a page-aligned region of memory for use as a thread's stack.
        NOTE: this returns the "top" of the stack, or the lower address.

        @param tid      Thread ID of the stack.

        @return (vaddr, page aligned size) tuple
        """
        assert(tid >= 0 and tid < SystemConfig.MAX_USER_THREADS)

        # Stack pages are allocated like this, where X is the guard page
        # and Y is the stack page
        # low          high
        #     XYXYXYXY
        ret = cls.VSPACE_STACK_LOW + (tid * cls.VSPACE_STACK_SIZE * 2) + cls.VSPACE_STACK_SIZE
        assert(ret % cls.VSPACE_STACK_SIZE == 0) # ensure alignment
        return ret, cls.VSPACE_STACK_SIZE

    @classmethod
    def alloc_vspace_tls(cls, is_threaded: bool, single_tls_size: int, single_tls_align: int) -> Tuple[int, int]:
        """
        Allocate memory region for the TLS in the PD VSpace.

        @param is_threaded      Is this PD multi-threaded?
        @param single_tls_size  Size of a single TLS region.
        @param single_tls_align Desired alignment of the single TLS region.

        @return (vaddr, aligned_size) tuple
        """
        # We make the start of the TLS memory area page-aligned, so check
        # that this plays well with the TLS alignment.
        assert(kb(4) % single_tls_align == 0)
        
        start = cls.VSPACE_TLS_START

        # Aligned size of a single TLS region (for one thread)
        aligned_size = round_up(single_tls_size, single_tls_align)
        if is_threaded:
            aligned_size *= SystemConfig.MAX_USER_THREADS

        # Page align
        aligned_size = round_up(aligned_size, kb(4))
        assert(start + aligned_size <= cls.VSPACE_TLS_END)

        return start, aligned_size
    
    @classmethod
    def alloc_vspace_ipc_buffer(cls, is_threaded: bool) -> Tuple[int, int]:
        """
        Allocate all the IPC Buffers in the VSpace.

        @param is_threaded  Is this PD multi-threaded?

        @return (vaddr, size) tuple
        """
        start = cls.VSPACE_IPCBUFF_LOW
        if is_threaded:
            size = cls.VSPACE_IPCBUFF_HIGH - cls.VSPACE_IPCBUFF_LOW
        else:
            size = cls.VSPACE_IPCBUFF_SIZE
        
        assert(start + size <= cls.VSPACE_STACK_LOW)
        return start, size
