=== libstupidcrt ===

An attempt at a dummy C runtime for seL4CP.

Objectives:
- To provide an alternative to the event-based model of seL4CP
- Support threading
- Create a minimal TLS, just for aarch64.
- Be completely backwards compatible with the sel4cp tool. The tool should
  not need to know what it is loading. Just needs to be an ELF file.
