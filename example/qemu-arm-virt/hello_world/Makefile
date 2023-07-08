#
# Copyright 2021, Breakaway Consulting Pty. Ltd.
#
# SPDX-License-Identifier: BSD-2-Clause
#
ifeq ($(strip $(BUILD_DIR)),)
$(error BUILD_DIR must be specified)
endif

ifeq ($(strip $(SEL4CP_SDK)),)
$(error SEL4CP_SDK must be specified)
endif

ifeq ($(strip $(SEL4CP_BOARD)),)
$(error SEL4CP_BOARD must be specified)
endif

ifeq ($(strip $(SEL4CP_CONFIG)),)
$(error SEL4CP_CONFIG must be specified)
endif

# List of partition directories
PARTITIONS := hello world

# Idea right now is to invoke the sel4cp tool on the root level .system
# file of the project (hello_world.system in this case). The sel4cp tool
# will assume that the project elfs are already compiled (done by this Makefile
# recursively calling make). The system initializer will have to take these elfs,
# load them onto seL4_TCBs, and assign them to domain. This process is automated by
# the tool, I guess.
all: images hello_world.system
	@echo $(BUILD_DIR)
	@echo $(SEL4CP_SDK)
	@echo $(SEL4CP_BOARD)
	@echo $(SEL4CP_CONFIG)

	$(SEL4CP_TOOL) hello_world.system --search-path $(BUILD_DIR) --board $(SEL4CP_BOARD) --config $(SEL4CP_CONFIG)

images:
	mkdir -p $(addprefix $(BUILD_DIR)/, $(PARTITIONS))
	$(MAKE) -C hello
	$(MAKE) -C world
