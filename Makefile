BUILD_DIR := build

include $(N64_INST)/include/n64.mk

TARGET := snake
ROM := $(TARGET).z64
ELF := $(BUILD_DIR)/$(TARGET).elf
DFS := $(BUILD_DIR)/$(TARGET).dfs

INC_DIRS := include include/engine
H_FILES := $(foreach dir,$(INC_DIRS),$(wildcard $(dir)/*.h))
SRC_DIRS := src src/engine
C_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
O_FILES := $(C_FILES:%.c=$(BUILD_DIR)/%.o)

N64_CFLAGS += $(INC_DIRS:%=-I%)

ASSETS_PNG := $(wildcard assets/*.png)
ASSETS_GLB := $(wildcard assets/*.glb)
ASSETS_CONV := $(ASSETS_PNG:assets/%.png=filesystem/%.sprite) \
               $(ASSETS_GLB:assets/%.glb=filesystem/%.t3dm)

final: $(ROM)
$(ROM): N64_ROM_TITLE="Snake 64"
$(ROM): $(DFS)
$(DFS): $(ASSETS_CONV)
$(ROM): $(ELF)
$(ELF): $(O_FILES)

filesystem/%.sprite: assets/%.png
	@mkdir -p $(dir $@)
	@echo "    [SPRITE] $@"
	$(N64_MKSPRITE) -o filesystem -c 3 $<

filesystem/%.t3dm: assets/%.glb
	@echo "    [T3D-MODEL] $@"
	$(T3D_GLTF_TO_3D) "$<" $@
	$(N64_MKASSET) -o filesystem -c 3 $@

format: $(H_FILES) $(C_FILES)
	clang-format --style=file -i $^
