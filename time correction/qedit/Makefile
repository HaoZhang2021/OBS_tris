########################################################################
TARGET	= qedit

SRCS	= $(TARGET).c bounds.c execute.c do.c \
	get_block_info.c get_stream_info.c parse_cmd.c parse_cmdline.c \
	position_stream.c qio.c store_block.c write_file.c \
	epoch.c get_rate.c timecvt.c

OBJS	= $(SRCS:%.c=%.o)

IQLIB2	= -I../qlib2


QLIB2	= ../qlib2/libqlib2nl-64.a


RLLIB	= -lreadline -ltermcap
RLFLAGS	= -I/usr/local/include -DUSE_READLINE

LIBS	= $(QLIB2) -lm $(RLLIB)

.PRECIOUS:	$(QLIB2)

BINDIR	= /usr/local/bin
MANDIR	= /usr/local/man
MANEXT	= 1

CC	= cc
COPT	= -g
CFLAGS	= $(COPT) $(IQLIB2) $(RLFLAGS) $(CARCH)

########################################################################

all:	$(TARGET) 

$(TARGET):	$(OBJS) $(QLIB2)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

install:	install_$(TARGET) 

install_$(TARGET):	$(TARGET)
	cp $(TARGET) $(BINDIR)/$(TARGET)

install_man:	$(TARGET).man 
	cp $(TARGET).man $(MANDIR)/man$(MANEXT)/$(TARGET).$(MANEXT)

clean:	
	rm -rf *.o

veryclean:	clean
	rm -rf $(TARGET) 

tar:	$(TARGET)
	(cd ..; tar cf - $(TARGET)/*.c $(TARGET)/*.h $(TARGET)/Makefile \
		$(TARGET)/*.man $(TARGET)/$(TARGET) \
		$(TARGET)/README $(TARGET)/CHANGES ) \
	| gzip > /tmp/$(TARGET).tar.gz

$(QLIB2):	FORCE
	(cd ../qlib2; make)

FORCE:

cleandepend:
	makedepend

depend:
	makedepend $(CFLAGS) *.c

########################################################################
# DO NOT DELETE THIS LINE -- make depend depends on it.
