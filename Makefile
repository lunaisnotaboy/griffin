OBJS = griffin.o pvr-texture.o romdisk.o
TARGET = griffin.elf

all: rm-elf $(TARGET)

include $(KOS_BASE)/Makefile.rules

$(TARGET): $(OBJS)
	kos-cc -o $(TARGET) $(OBJS) -L$(KOS_BASE)/lib -lGL -lpcx -lkosutils

clean: rm-elf
	-rm -f $(OBJS)

dist: $(TARGET)
	-rm -f $(OBJS) romdisk.img
	$(KOS_STRIP) $(TARGET)

rm-elf:
	-rm -f $(TARGET) romdisk.*

romdisk.img:
	$(KOS_GENROMFS) -f romdisk.img -d romdisk -v

romdisk.o: romdisk.img
	$(KOS_BASE)/utils/bin2o/bin2o romdisk.img romdisk romdisk.o

run: $(TARGET)
	$(KOS_LOADER) $(TARGET)
