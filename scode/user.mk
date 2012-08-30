CORE = A7
CODE = dbg

CHIP = NIKED3
OBJFLASH = SPI

ifeq ($(CHIP),NIKED3)
    UFLAG = -DAML_NIKED3
endif

ifeq ($(OBJFLASH),NAND)
    UFLAG += -DNAND
endif

ifeq ($(OBJFLASH),SPI)
    UFLAG += -DSPI
endif

TARGET = $(CHIP)_$(OBJFLASH)_ERASE
