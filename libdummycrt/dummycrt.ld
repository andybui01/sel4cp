/*
 * Copyright 2021, Breakaway Consulting Pty. Ltd.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
PHDRS
{
    text PT_LOAD;
    data PT_LOAD;

    /* TLS segments are read-only though they are write-able because
     * this is the initial copy (with initialied variables). We copy
     * this into each thread's TLS to allow them to modify. */
    tls PT_TLS;
}

ENTRY(sel4cp_thread_entry);

STARTUP(crt0.o);

SECTIONS
{
    . = 0x200000;

    .text :
    {
        _text = .;
        *(.text.start)
        *(.text*)
        *(.rodata)
        _text_end = .;
    } :text

    . = ALIGN(0x1000);

    /* For some reason the ABI puts init array
     * into writable memory, so we have to follow suit */
    .init_array :
    {
        PROVIDE(__init_array_start = .);
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array*))
        PROVIDE(__init_array_end = .);
    } :data

    .data :
    {
        _data = .;
        *(.data)
        _data_end = .;
    } :data

    .tdata :
    {
        PROVIDE( __tdata_start = . );
        *(.tdata .tdata.* .gnu.linkonce.td.*)
        PROVIDE( __tdata_end = . );
    } :tls

    .tbss :
    {
        *(.tbss .tbss.* .gnu.linkonce.tb.*)
        *(.tcommon .tcommon.*)
        PROVIDE( __tbss_end = . );
    } :tls

    .bss :
    {
        _bss = .;
        *(.bss)
        *(COMMON)
        . = ALIGN(4);
        _bss_end = .;
    } :data
}
