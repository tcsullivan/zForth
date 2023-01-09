
/*
 * Example zforth main app for msp430.
 */

#include <msp430g2553.h>

#include <ctype.h>
#include <stdint.h>

#include "zforth.h"

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

int main(void)
{
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

    static const char *zforth = "zforth\n\r";
    serputs(zforth);

    char buf[16];
    char *ptr = buf;
    for (;;) {
        if (IFG2 & UCA0RXIFG) {
            char c = UCA0RXBUF;
            serput(c);

            if (c == '\r') {
                *ptr = '\0';

                zf_result r = zf_eval(buf);
                serputs(zforth + 6);
                serput((r == ZF_OK) ? 'O' : 'E');

                ptr = buf;
            } else {
                *ptr++ = c;
            }
        }
    }

    //zf_eval(": . 1 sys ;");
}


zf_input_state zf_host_sys(zf_syscall_id id, const char *input)
{
    switch((int)id) {

        case ZF_SYSCALL_EMIT:
            serput(zf_pop());
            break;

        //case ZF_SYSCALL_PRINT:
        //    itoa(zf_pop(), buf, 10);
        //    puts(buf);
        //    break;
    }

    return 0;
}


zf_cell zf_host_parse_num(const char *buf)
{
    zf_cell n = 0;
    int c;

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

    return n;
}


/*
 * End
 */

