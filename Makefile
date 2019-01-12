##
## Makefile for yogurt maker for STM8/W1209
##

## Build variables
ProjectName            := yogurtmaker

DEVICE                 := stm8
CHIP                   := STM8S003F3P6


##
## Common variables
## CC and CFLAGS can be overriden using an environment variables
##
CC           := sdcc-sdcc
LD           := sdcc-sdcc
MKDIR        := mkdir -p

INCLUDE      := -I. -I./include
BUILD        := ./Build
TARGET       := $(BUILD)/$(ProjectName).ihx

CFLAGS       := -l $(DEVICE) -m$(DEVICE) $(INCLUDE)
LDFLAGS      := --out-fmt-ihx -m$(DEVICE)
ObjectSuffix := .rel

##
## User defined environment variables
##
SRCS := ym.c display.c timer.c buttons.c adc.c menu.c params.c relay.c
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

##
## Clean
##
clean:
	$(RM) -r $(BUILD)/


