##
## Makefile for yogurt maker for STM8/W1209
##

## Build variables
ProjectName            := yogurtmaker

DEVICE                 := stm8
CHIP                   := STM8S003F3P6

## Configuration
CONFIG := # CONFIG_ENABLE_FULL_UPTIME  CONFIG_USE_RELAY_BUZZ  RIGHT_ALIGN_TEXT

##
## Common variables
## CC and CFLAGS can be overriden using an environment variables
##
MKDIR        := mkdir -p
INCLUDE      := -I. -I./include
BUILD        := ./Build

ifeq ($(GCC),1)
CC           := gcc
LD           := gcc
TARGET       := $(BUILD)/$(ProjectName)

CFLAGS       := $(INCLUDE) -Wall -O2 -D'WAIT_FOR_INTERRUPT()=' -D'INTERRUPT_ENABLE()=' -D'__interrupt(ARGS...)='
LDFLAGS      :=
ObjectSuffix := .o
else
CC           := sdcc-sdcc
LD           := sdcc-sdcc
TARGET       := $(BUILD)/$(ProjectName).ihx

CFLAGS       := -l $(DEVICE) -m$(DEVICE) $(INCLUDE)
LDFLAGS      := --out-fmt-ihx -m$(DEVICE)
ObjectSuffix := .rel
endif

##
## User defined environment variables
##
SRCS := ym.c display.c timer.c buttons.c adc.c menu.c params.c relay.c persist.c
OBJS := $(SRCS:%=$(BUILD)/%$(ObjectSuffix))


##
## Main Build Targets
##
.PHONY: all clean

all: $(BUILD)/ $(TARGET)

$(BUILD)/:
	@$(MKDIR) $(@D)

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^


##
## Objects
##
$(BUILD)/%.c$(ObjectSuffix): %.c
	$(CC) $(CFLAGS) -c $< -o $@

##
## Flash management targets
##
unlock:
	stm8flash -c stlinkv2 -p $(CHIP) -u
flash:
	stm8flash -c stlinkv2 -p $(CHIP)  -w $(TARGET)

size:
	for i in Build/*.rel; do \
		echo $$(sed -n 's/^.* CODE size \([^ ]*\).*$$/0x\1/p' < $$i) $${i%.rel}; \
	done

##
## Clean
##
clean:
	$(RM) -r $(BUILD)/


