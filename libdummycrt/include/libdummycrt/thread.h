#pragma once

void sel4cp_thread_entry(void *ipc_buffer, void *tls_memory, int (*entry)(void));
