## zForth for the MSP430

This is a customized port of zForth for the MSP430, a 16-bit microcontroller with minimal memory.

This specifically targets the MSP430G2553 or other MSP430 family members with at least 512 bytes of RAM and 8 kilobytes of ROM.

Satisfying these constraints primarily required adding a read-only dictionary that contains the bootstrap, core functionality (modified to better conform with ANS Forth), and other helpful words. MSP430 registers can also be referenced by name.

Through these changes, zForth is made available over UART with over 256 bytes of dictionary RAM and 10 cells each for the data and return stacks.

```
: led 1 p1out ;
: btn 8 p1in ;

\ Configure LED (P1.0) for output:
1 p1dir set

\ Plan LED response to button:
: act if set else clr then ;

\ Do a little loop updating the LED:
: act begin led btn tst act again ;
act

```

---

To update the read-only dictionary:

1. Make changes to core.zf.
2. Copy core.zf and zfconf.h over to another zForth build that supports dictionary saving (e.g. the Linux example).
3. In zfconf.h, Set `ZF_ENABLE_PREBUILT_BOOTSTRAP` to 0, `ZF_ENABLE_BOOTSTRAP` to 1, and increase `ZF_DICT_SIZE` to accommodate the final dictionary.
4. Build zForth and run: `./zforth core.zf`.
5. Save the dictionary and note new "latest" index: `131 sys latest @ .`.
6. Convert dictionary to C header: `xxd -i zforth.save > core.h`; copy core.h back into the MSP430 build.
7. Update core.h by adding the line `unsigned int zforth_save_latest = <the new "latest" index>;`, removing excess zero bytes from `zforth_save`, and updating `zforth_save_len` to reflect the new length. Also, make all variables `static const`.
8. Compile the MSP430 build and test.

