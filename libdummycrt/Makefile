ifeq ($(strip $(BUILD_DIR)),)
$(error BUILD_DIR must be specified)
endif

ifeq ($(strip $(GCC_CPU)),)
$(error GCC_CPU must be specified)
endif

TOOLCHAIN := aarch64-none-elf-
CFLAGS := -std=gnu11 -g3 -O3 -nostdlib -ffreestanding -mcpu=$(GCC_CPU) -Wall -Wno-maybe-uninitialized -Wno-unused-function -Werror -I$(SEL4_SDK)/include -Iinclude

LIBS := libdummycrt.a
OBJS := crt1.o thread.o tls.o string.o
# This is mainly for assembly files, but for dummycrt we want to make
# the linker script available to the other libs too.
OTHER_OBJS :=  crt0.o dummycrt.ld

$(BUILD_DIR)/%.o : src/%.S
	$(TOOLCHAIN)gcc -x assembler-with-cpp -c -g  -mcpu=$(GCC_CPU)  $< -o $@

$(BUILD_DIR)/%.o : src/%.s
	$(TOOLCHAIN)as -g -mcpu=$(GCC_CPU) $< -o $@

$(BUILD_DIR)/%.o : src/%.c
	$(TOOLCHAIN)gcc -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.ld : %.ld
	cp $< $@

LIB = $(addprefix $(BUILD_DIR)/, $(LIBS))

all: $(LIB) $(addprefix $(BUILD_DIR)/, $(OTHER_OBJS))

$(LIB): $(addprefix $(BUILD_DIR)/, $(OBJS))
	$(TOOLCHAIN)ar -rv $@ $^
