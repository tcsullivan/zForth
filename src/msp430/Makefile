
BIN	:= zforth-msp430
SRC	:= main.c zforth.c

HEX	:= $(BIN).hex
OBJS    := $(subst .c,.o, $(SRC))
DEPS    := $(subst .c,.d, $(SRC))

CC	:= msp430-elf32-gcc

VPATH   := ../zforth
CFLAGS	+= -I. -I../zforth

CFLAGS	+= -mmcu=msp430g2553
CFLAGS	+= -Os -g -std=c11 -MMD
CFLAGS	+= -Wall -Wextra -Werror -Wno-unused-parameter -Wno-clobbered -Wno-unused-result
CFLAGS	+= -ffunction-sections -fdata-sections

LDFLAGS	+= -g
LDFLAGS	+= -mmcu=msp430g2553
LDFLAGS	+= -Wl,-gc-sections

CFLAGS  += -flto
LDFLAGS += -flto

$(HEX): $(BIN)
	msp430-elf32-objcopy -j .text -j .data -O ihex $< $@
	msp430-elf32-size $<

$(BIN): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	rm -f $(HEX) $(BIN) $(OBJS) $(DEPS)

-include $(DEPS)

