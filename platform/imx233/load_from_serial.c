#include "serial.h"
#include "utils/str.h"
#include "stdbool.h"


#define BUFF_SIZE 256
#define START_DELIM 'S'
#define END_DELIM 'E'
#define START_ADDR 0X40900000

static void halt() {while (true);}

static bool validate_checksum(unsigned char* buff, unsigned int csm)
{
    int i;
    unsigned int s = 0;

    for (i=0; i<BUFF_SIZE; i+=4) {
        s += *(unsigned int*)(buff+i);
    }

    return s==csm;
}


void load_from_serial()
{
    unsigned char buff[BUFF_SIZE];
    unsigned short num_blocks;
    unsigned int start_addr;
    unsigned short curr_block = 0;

    delay(1);
    
    if (!serial_tstc()) {
        serial_puts("no char in buffer, returning to normal boot\n");
        return;
    }
    if ((char)serial_getc() != 0x02) {
        serial_puts("first char mismatch, returning to normal boot\n");
        return;
    }
    if ((char)serial_getc() != 0x03) {
        serial_puts("second char mismatch, returning to normal boot\n");
        return;
    }
    if ((char)serial_getc() != 0x03) {
        serial_puts("third char mismatch, returning to normal boot\n");
        return;
    }

    // number of blocks
    num_blocks = (unsigned short)serial_getc();
    num_blocks += (unsigned short)serial_getc() * 0x100;

    // start address
    start_addr = (unsigned short)serial_getc() +
                 (unsigned short)serial_getc() * 0x100 +
                 (unsigned short)serial_getc() * 0x10000 +
                 (unsigned short)serial_getc() * 0x1000000;

    while (curr_block < num_blocks) {
        unsigned int csm;
        unsigned int i;

        // start delimiter
        if (serial_getc() != START_DELIM) {
            serial_puts("wrong start delimiter\n");
            halt();
        }

        for (i=0; i<BUFF_SIZE; i++) {
            buff[i] = (unsigned char)serial_getc();
        }

        // end delimiter
        if (serial_getc() != END_DELIM) {
            serial_puts("wrong end delimiter\n");
            halt();
        }

        // checksum
        csm = (unsigned short)serial_getc() +
              (unsigned short)serial_getc() * 0x100 +
              (unsigned short)serial_getc() * 0x10000 +
              (unsigned short)serial_getc() * 0x1000000;

        if (!validate_checksum(buff, csm)) {
            serial_puts("wrong checksum\n");
            halt();
        }

        memcpy((char*)START_ADDR + BUFF_SIZE*curr_block, buff, BUFF_SIZE);

        // TODO: indicate we received the buffer and copied it

        curr_block++;
    }

    ((void (*)(void))start_addr)();
}

