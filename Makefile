TARGET := psposk2
INSTALL_PATH := /usr/src/busybox/_install/usr/bin

IMAGES = Eng.bmp EngActive.bmp Cap.bmp CapActive.bmp Num.bmp NumActive.bmp Mouse.bmp
OBJS = oskmain.o osk.o oskstates.o osk_psp.o $(IMAGES:%.bmp=%.o)

CC := mipsel-linux-gcc
CXX := mipsel-linux-g++
HOSTCC := gcc
BMP2C := bmp2c
CFLAGS = -fno-jump-tables
CXXFLAGS = -fno-jump-tables
LDFLAGS = -Wl,-elf2flt -static


.PHONY: all
all: $(TARGET)
	@echo "*** Done ***"

.PHONY: install
install: $(TARGET)
	cp $(TARGET) $(INSTALL_PATH)/$(TARGET)
	@echo "*** Done ***"

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) $^ -o $@

$(BMP2C): $(BMP2C).c
	$(HOSTCC) $< -o $@

.SECONDARY: $(IMAGES:%.bmp=%.c)

%.c: %.bmp $(BMP2C) oskimg.h
	$(BMP2C) $<

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@


# Dependencies
osk.o: osk.cpp osk.h oskstates.h
oskmain.o: oskmain.cpp osk.h oskstates.h
osk_psp.o: osk_psp.cpp osk.h oskstates.h oskimg.h
oskstates.o: oskstates.cpp osk.h oskstates.h
bmp2c.o: bmp2c.c oskimg.h


.PHONY: clean
clean:
	rm -f $(TARGET) $(BMP2C) *.o *.gdb $(IMAGES:%.bmp=%.c)
