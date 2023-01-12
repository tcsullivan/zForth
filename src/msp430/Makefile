
BIN	:= zforth-msp430
SRC	:= main.c zforth.c crt0.S

OBJS    := $(subst .S,.o, $(subst .c,.o, $(SRC)))
DEPS    := $(subst .S,.d, $(subst .c,.d, $(SRC)))

CC	:= msp430-elf32-gcc
AS	:= msp430-elf32-as

VPATH   := ../zforth
CFLAGS	+= -I. -I../zforth

AFLAGS	+= -mmcu=msp430g2553

CFLAGS	+= -mmcu=msp430g2553
CFLAGS	+= -Os -g -std=c99 -MMD
CFLAGS	+= -Wall -Wextra -Werror -pedantic
CFLAGS	+= -ffunction-sections -fdata-sections

LDFLAGS	+= -g
LDFLAGS	+= -mmcu=msp430g2553
LDFLAGS	+= -Wl,-gc-sections -nostartfiles

CFLAGS  += -flto -fstack-usage
LDFLAGS += -flto -fstack-usage

$(BIN): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
	size $@

.S.o:
	$(AS) $(AFLAGS) -c $< -o $@

clean:
	rm -f $(BIN) $(OBJS) $(DEPS)

-include $(DEPS)

