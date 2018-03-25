#-----------------------------------------------------------------------------------------------
 rwildcard = $(foreach d, $(wildcard $1*), $(filter $(subst *, %, $2), $d) $(call rwildcard, $d/, $2))

 ifeq ($(strip $(DEVKITARM)),)
 $(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
 endif

 include $(DEVKITARM)/base_tools

 name := Rei-Six

#-----------------------------------------------------------------------------------------------
 dir_source	  := source
 dir_include  := include
 dir_common	  := common
 dir_build	  := build
 dir_loader	  := loader
 dir_out	  := out
 dir_payload  := payloads
 dir_arm11    := arm11
#-----------------------------------------------------------------------------------------------

ASFLAGS := -mlittle-endian -mcpu=arm946e-s
CFLAGS  := -Wall -Wextra $(ASFLAGS) -fno-builtin -std=c11 -Wno-main -O2 -flto -ffast-math
CFLAGS	+= -I$(dir_include)

objects_cfw = $(patsubst $(dir_source)/%.s, $(dir_build)/%.o, \
			  $(patsubst $(dir_source)/%.c, $(dir_build)/%.o, \
			  $(call rwildcard, $(dir_source), *.s *.c)))
              
payloads = $(dir_build)/emunand.bin.o $(dir_build)/reboot.bin.o
              
define bin2o
	bin2s $< | $(AS) -o $(@)
endef
			  
#-----------------------------------------------------------------------------------------------

.PHONY: all
all: sighax loader

.PHONY: sighax
sighax: $(dir_out)/boot.firm

.PHONY: loader
loader: $(dir_out)/rei/loader.cxi

.PHONY: clean
clean:
	@$(MAKE) $(FLAGS) -C $(dir_loader) clean
	@$(MAKE) $(FLAGS) -C $(dir_arm11) clean
	rm -rf $(dir_out) $(dir_build)

#-----------------------------------------------------------------------------------------------

PHONY: $(dir_out)/boot.firm
$(dir_out)/boot.firm:  $(dir_build)/arm11.elf $(dir_build)/main.elf
	@mkdir -p "$(@D)"
	@firmtool build $@ -D $^ -A 0x18180000 0x18000000 -C XDMA NDMA -i
	
#-----------------------------------------------------------------------------------------------

$(dir_build)/main.elf: $(payloads) $(objects_cfw)
	$(LINK.o) -T linker.ld $(OUTPUT_OPTION) $^

$(dir_build)/arm11.elf: $(dir_arm11)
	@mkdir -p "$(@D)"
	@$(MAKE) -C $<

#-----------------------------------------------------------------------------------------------

$(dir_out)/rei/: $(dir_common)/top_splash.bin $(dir_common)/bottom_splash.bin
	@mkdir -p "$(dir_out)/rei"
	@cp -av $^ $@

$(dir_out)/rei/patches/patches.dat: $(wildcard common/patches/*.rnp)
	@mkdir -p "$(@D)"
	cat $^ > $@

$(dir_out)/rei/loader.cxi: $(dir_loader) $(dir_out)/rei $(dir_out)/rei/patches/patches.dat
	@$(MAKE) $(FLAGS) -C $(dir_loader)
	@mv $(dir_loader)/loader.cxi $(dir_out)/rei

$(dir_build)/%.bin: $(dir_payload)/%.s
	@mkdir -p "$(@D)"
	@armips $<

$(dir_build)/%.bin.o: $(dir_build)/%.bin
	@$(bin2o)

$(dir_build)/payloads.h: $(payloads)
	@$(foreach f, $(payloads),\
	echo "extern const u8" `(echo $(basename $(notdir $(f))) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"[];" >> $@;\
	echo "extern const u32" `(echo $(basename $(notdir $(f)))| sed -e 's/^\([0-9]\)/_\1/' | tr . _)`_size";" >> $@;\
    )

#-----------------------------------------------------------------------------------------------

$(dir_build)/%.o: $(dir_source)/%.c $(dir_build)/payloads.h
	@mkdir -p "$(@D)"
	$(COMPILE.c) $(OUTPUT_OPTION) $<

$(dir_build)/%.o: $(dir_source)/%.s
	@mkdir -p "$(@D)"
	$(COMPILE.s) $(OUTPUT_OPTION) $<
include $(call rwildcard, $(dir_build), *.d)

#-----------------------------------------------------------------------------------------------