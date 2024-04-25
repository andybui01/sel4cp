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

ifeq ($(strip $(SEL4CP_TOOL)),)
$(error SEL4CP_TOOL must be specified)
endif

IMAGE_FILE = $(BUILD_DIR)/loader.img
REPORT_FILE = $(BUILD_DIR)/report.txt

PD_LIST := hello world os timeout

all: images hello_world.system
	@echo ===Makefile params===
	@echo $(BUILD_DIR)
	@echo $(SEL4CP_SDK)
	@echo $(SEL4CP_BOARD)
	@echo $(SEL4CP_CONFIG)
	@echo $(SEL4CP_TOOL)
	@echo 
	@echo ===Running the tool===
	$(SEL4CP_TOOL) hello_world.system --search-path $(BUILD_DIR) --board $(SEL4CP_BOARD) --config $(SEL4CP_CONFIG) -o $(IMAGE_FILE) -r $(REPORT_FILE)

images:
	mkdir -p $(addprefix $(BUILD_DIR)/, $(PD_LIST))
	$(MAKE) -C hello
	$(MAKE) -C world
	$(MAKE) -C os
	$(MAKE) -C timeout
