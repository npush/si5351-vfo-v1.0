MCU     = atmega328p # see `make show-mcu`
OSC     = 16000000UL
PROJECT = si5351-vfo
BUILDDIR = build

# ----- These configurations are quite likely not to be changed -----

# Binaries
GCC     = avr-gcc
G++     = avr-g++
SIZE	= avr-size
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
RM      = rm -f
AVRDUDE = avrdude

# Files
EXT_C   = c
EXT_C++ = cpp
EXT_ASM = S

# ----- No changes should be necessary below this line -----

SOURCEDIR = 

INCLUDES =  \
		-Iarduino \
		-Iarduino/lib \
		-Iarduino/lib/Wire \
		-Iarduino/lib/Wire/utility \
		-Iarduino/lib/LC \
		-Iarduino/lib/EEPROM

C_SOURCES = 		$(shell find -L $(SOURCEDIR) -name '*.c')
CPP_SOURCES =	$(shell find -L $(SOURCEDIR) -name '*.cpp')
ASM_SOURCES =	$(shell find -L $(SOURCEDIR) -name '*.S')


OBJECTS = $(addprefix $(BUILDDIR)/, $(addsuffix .o, $(basename $(C_SOURCES))))
OBJECTS += $(addprefix $(BUILDDIR)/, $(addsuffix .o, $(basename $(CPP_SOURCES))))
OBJECTS += $(addprefix $(BUILDDIR)/, $(addsuffix .o, $(basename $(ASM_SOURCES))))

#OBJECTS = \
#	$(patsubst %.$(EXT_C),%.o,$(wildcard *.$(EXT_C))) \
#	$(patsubst %.$(EXT_C++),%.o,$(wildcard *.$(EXT_C++))) \
#	$(patsubst %.$(EXT_ASM),%.o,$(wildcard *.$(EXT_ASM)))

# TODO explain these flags, make them configurable
CFLAGS = $(INCLUDES)
CFLAGS += -Os -w -ffunction-sections -fdata-sections
CFLAGS += -MMD
CFLAGS +=-DARDUINO_AVR_NANO -DARDUINO_ARCH_AVR
CFLAGS += -DF_CPU=$(OSC)
CFLAGS += -mmcu=$(MCU)

C++FLAGS = $(INCLUDES)
C++FLAGS += -Os -w  -ffunction-sections -fdata-sections
C++FLAGS += -fno-exceptions -fno-threadsafe-statics
C++FLAGS += -MMD
C++FLAGS += -DARDUINO_AVR_NANO -DARDUINO_ARCH_AVR
C++FLAGS += -DF_CPU=$(OSC)
C++FLAGS += -mmcu=$(MCU)

ASMFLAGS = $(INCLUDES)
ASMFLAGS += -Os
ASMFLAGS += -Wall -Wstrict-prototypes
ASMFLAGS += -DF_CPU=$(OSC)
ASMFLAGS += -mmcu=$(MCU)
ASMFLAGS += -x assembler-with-cpp
ASMFLAGS += -flto -MMD -DARDUINO_AVR_NANO -DARDUINO_ARCH_AVR

LDFLAGS += -mmcu=$(MCU) -Wl,--gc-sections
LDFLAGS += -w -Os -lm

default: $(BUILDDIR)/$(PROJECT).elf


$(BUILDDIR)/$(PROJECT).elf: $(OBJECTS)
	$(GCC) $(LDFLAGS) -o $@ $(OBJECTS)
	 #$^ $(LDLIBS) -o $@


$(BUILDDIR)/%.o : %.$(EXT_C)
	mkdir -p $(dir $@)
	$(GCC) $< $(CFLAGS) -c -o $@

$(BUILDDIR)/%.o : %.$(EXT_C++)
	mkdir -p $(dir $@)
	$(G++) $< $(C++FLAGS) -c -o $@

$(BUILDDIR)/%.o : %.$(EXT_ASM)
	mkdir -p $(dir $@)
	$(GCC) $< $(ASMFLAGS) -c -o $@

clean:
	$(RM) $(BUILDDIR)/$(PROJECT).elf $(BUILDDIR)/$(OBJECTS)

size:
	$(SIZE) $(BUILDDIR)/$(PROJECT).elf


hex: $(BUILDDIR)/$(PROJECT).elf
	rm -f $(BUILDDIR)/$(PROJECT).hex
	$(OBJCOPY) -j .text -j .data -O ihex $(BUILDDIR)/$(PROJECT).elf $(BUILDDIR)/$(PROJECT).hex
	$(SIZE) $(BUILDDIR)/$(PROJECT).elf

help:
	@echo "usage:"
	@echo "  make <target>"
	@echo ""
	@echo "targets:"
	@echo "  clean     Remove any non-source files"
	@echo "  config    Shows the current configuration"
	@echo "  help      Shows this help"
	@echo "  show-mcu  Show list of all possible MCUs"

config:
	@echo "configuration:"
	@echo ""
	@echo "Binaries for:"
	@echo "  C compiler:   $(GCC)"
	@echo "  C++ compiler: $(G++)"
	@echo "  Programmer:   $(AVRDUDE)"
	@echo "  remove file:  $(RM)"
	@echo ""
	@echo "Hardware settings:"
	@echo "  MCU: $(MCU)"
	@echo "  OSC: $(OSC)"
	@echo ""
	@echo "Defaults:"
	@echo "  C-files:   *.$(EXT_C)"
	@echo "  C++-files: *.$(EXT_C++)"
	@echo "  ASM-files: *.$(EXT_ASM)"

show-mcu:
	$(G++) --help=target

deb:
#	@echo $(INCLUDES)
#	@echo $(SOURCES)
	@echo $(OBJECTS)