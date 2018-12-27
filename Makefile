##
## Slightly corrected version of auto generated makefile by CodeLite IDE
##
## Build variables
ProjectName            := yogurtmaker
Device                 := stm8

TARGET                 := $(ProjectName).ihx
SourceDirectory        := .
BUILD                  := ./Build

##
## Common variables
## CC and CFLAGS can be overriden using an environment variables
##
CC           := sdcc-sdcc
LD           := sdcc-sdcc
MKDIR        := mkdir -p
INCLUDE      := -I. -I./include
CFLAGS       := -l$(Device) -m$(Device) $(INCLUDE)
LDFLAGS      := --out-fmt-ihx -m$(Device)
ObjectSuffix := .rel

##
## User defined environment variables
##
Source := ym.c display.c timer.c buttons.c adc.c menu.c params.c relay.c
Objects := $(Source:%=$(BuildDirectory)/%$(ObjectSuffix))


##
## Main Build Targets
##
.PHONY: all clean
all: $(BUILD)/$(TARGET)

$(BUILD)/$(TARGET): $(Objects)
	@$(MKDIR) $(@D)
	$(LD) $(LDFLAGS) -o$(OutputFile) $(Objects)


##
## Objects
##
$(BuildDirectory)/%.c$(ObjectSuffix): $(SourceDirectory)/%.c
	$(CC) $(CFLAGS) -c $< -o$@


##
## Clean
##
clean:
	$(RM) -r $(BuildDirectory)/


