TARGET = $(BEREMIZ_BUILD_PATH)/prog

#to use in Beremiz: make -C ~/toymatic/runtime_src BEREMIZ_BUILD_PATH=%(buildpath)s 

all: $(TARGET).bin
# all : prog

PREFIX:=riscv64-unknown-elf-

CFLAGS += -I/usr/include/newlib/ -I$(HOME)/Beremiz/matiec/lib/C  -I$(BEREMIZ_BUILD_PATH)
CFLAGS +=-Os -march=rv32im_zicsr -mabi=ilp32 -nostdlib -static-libgcc -ffunction-sections -fdata-sections

LDFLAGS:= -T flatfile.lds -Wl,--gc-sections -lgcc -flto

MAIN_SRC = main.c startup.c 
IEC_SRC = $(BEREMIZ_BUILD_PATH)/POUS.h $(BEREMIZ_BUILD_PATH)/POUS.c $(BEREMIZ_BUILD_PATH)/config.c $(BEREMIZ_BUILD_PATH)/resource1.c

$(TARGET).elf : $(MAIN_SRC) $(IEC_SRC)
	$(PREFIX)gcc -o $@ $(CFLAGS) $(LDFLAGS) $(MAIN_SRC)

$(TARGET).bin : $(TARGET).elf
	$(PREFIX)objcopy $^ -O binary $@

prog : $(TARGET).bin 
	toymatic_tool /dev/ttyUSB0 -p $^ -r

clean :
	rm $(TARGET).*
