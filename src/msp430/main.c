
/*
 * Example zforth main app for msp430.
 */

#include <msp430g2553.h>

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "zforth.h"

static const unsigned char *regs = (unsigned char *)
#include "regs_data.h"
    ;

static void serput(int c);
static void serputs(const char *s);
static void printint(int n, char *buf);

static zf_cell lookup_reg(const char *buf);
static zf_result do_eval(const char *buf);

int main(void)
{
    static char buf[64];

    WDTCTL = WDTPW | WDTHOLD;
    DCOCTL = 0;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    P1SEL |= BIT1 | BIT2;
    P1SEL2 |= BIT1 | BIT2;

    UCA0CTL1 = UCSWRST;
    UCA0CTL1 |= UCSSEL_2;
    UCA0BR0 = 104;
    UCA0BR1 = 0;
    UCA0MCTL = UCBRS0;
    UCA0CTL1 &= ~UCSWRST;

    zf_init(0);

    serputs("zforth   ");
    printint(ZF_DICT_SIZE, buf);
    serputs(" bytes\n\r");

    char *ptr = buf;
    for (;;) {
        if (IFG2 & UCA0RXIFG) {
            char c = UCA0RXBUF;
            serput(c);

            if (c == '\r') {
                *ptr = '\0';

                serputs("\n\r");
                do_eval(buf);
                serputs("\n\r");

                ptr = buf;
            } else if (c == '\b') {
                --ptr;
            } else if (ptr < buf + sizeof(buf)) {
                *ptr++ = c;
            }
        }
    }
}

void serput(int c)
{
    while (!(IFG2 & UCA0TXIFG));
    UCA0TXBUF = (char)c;
}

void serputs(const char *s)
{
    while (*s)
        serput(*s++);
}

void printint(int n, char *buf)
{
    char *ptr = buf;
    bool neg = n < 0;

    if (neg)
        n = -n;

    do {
        *ptr++ = n % 10 + '0';
    } while ((n /= 10));

    if (neg)
        serput('-');

    do {
        serput(*--ptr);
    } while (ptr > buf);
    serput(' ');
}

zf_input_state zf_host_sys(zf_syscall_id id, const char *input)
{
    static char buf[12];

    switch((int)id) {
        case ZF_SYSCALL_EMIT:
            serput(zf_pop());
            break;

        case ZF_SYSCALL_PRINT:
            printint(zf_pop(), buf);
            break;

        case ZF_SYSCALL_USER + 0: { // : peek8 128 sys ;
            // byte peek
            uint8_t *p = (uint8_t *)zf_pop();
            zf_push(*p);
            } break;

        case ZF_SYSCALL_USER + 1: { // : poke8 129 sys ;
            // byte poke
            uint8_t *p = (uint8_t *)zf_pop();
            *p = zf_pop();
            } break;
    }

    return 0;
}

zf_cell zf_host_parse_num_hex(const char *buf)
{
    zf_cell n = 0;
    int c;

    while ((c = *buf)) {
        if (isspace(c)) {
            break;
        } else if (c >= 'a' && c <= 'f') {
            c -= 32; // assume ascii...
            c -= 65 - 10;
        } else if (c >= 'A' && c <= 'F') {
            c -= 65 - 10;
        } else if (!isdigit(c)) {
            zf_abort(ZF_ABORT_NOT_A_WORD);
            n = 0;
            break;
        } else {
            c -= '0';
        }

        n = n * 16 + c;
        ++buf;
    }

    return n;
}

zf_cell zf_host_parse_num(const char *buf)
{
    zf_cell n = 0;
    int c;
    bool neg = false;

    if (isalpha((int)buf[0]))
	return lookup_reg(buf);
    else if (buf[0] == '0' && buf[1] == 'x')
        return zf_host_parse_num_hex(buf + 2);
    else if (buf[0] == '-')
        neg = true, buf = buf + 1;

    while ((c = *buf)) {
        if (isspace(c)) {
            break;
        } else if (!isdigit(c)) {
            zf_abort(ZF_ABORT_NOT_A_WORD);
            n = 0;
            break;
        }

        n = n * 10 + c - '0';
        ++buf;
    }

    return neg ? -n : n;
}

zf_cell lookup_reg(const char *buf)
{
    const unsigned char *ptr = regs;
    while (*ptr) {
	unsigned len = (unsigned)*ptr;

        if (memcmp(ptr + 1, buf, len) == 0 && buf[len] == '\0')
            return ptr[len + 1] | (ptr[len + 2] << 8);

        ptr += *ptr + 3;
    }

    zf_abort(ZF_ABORT_NOT_A_WORD);
    return 0;
}

zf_result do_eval(const char *buf)
{
	const char *msg = NULL;

	zf_result rv = zf_eval(buf);

	switch(rv)
	{
		case ZF_OK: break;
		case ZF_ABORT_INTERNAL_ERROR: msg = "internal error"; break;
		case ZF_ABORT_OUTSIDE_MEM: msg = "outside memory"; break;
		case ZF_ABORT_DSTACK_OVERRUN: msg = "dstack overrun"; break;
		case ZF_ABORT_DSTACK_UNDERRUN: msg = "dstack underrun"; break;
		case ZF_ABORT_RSTACK_OVERRUN: msg = "rstack overrun"; break;
		case ZF_ABORT_RSTACK_UNDERRUN: msg = "rstack underrun"; break;
		case ZF_ABORT_NOT_A_WORD: msg = "not a word"; break;
		case ZF_ABORT_COMPILE_ONLY_WORD: msg = "compile-only word"; break;
		case ZF_ABORT_INVALID_SIZE: msg = "invalid size"; break;
		case ZF_ABORT_DIVISION_BY_ZERO: msg = "division by zero"; break;
		default: msg = "unknown error";
	}

	if(msg) {
            serputs(msg);
            serputs("\r\n");
	}

	return rv;
}


/*
 * End
 */

