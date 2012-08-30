CFILE = spi.c nand.c main.c
HFILE = spi.h nand.h main.h
OBJ   = spi.o nand.o main.o

ifeq ($(CHIP),NIKED3)
    OTHERFILES = 8m_niked3.cmd
endif
