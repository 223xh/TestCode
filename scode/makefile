include $(CURDIR)/user.mk
include $(CURDIR)/files.mk

ifeq ($(CORE),A7)
    CC = mcc
else
    CC = hcarc
endif

ifeq ($(CODE),dbg)
    CFLAG = -g
endif

OTHERFLAGS = -zpurge

OUTFILE = $(TARGET).out

all: $(OUTFILE)

$(OUTFILE): $(CFILE) $(HFILE)
	$(CC) -c $(CFILE) $(CFLAG) $(UFLAG)
	$(CC) $(OBJ) $(OTHERFLAGS) $(OTHERFILES) -o $(OUTFILE)
clean:
	rm -rf $(OBJ) $(OUTFILE)
