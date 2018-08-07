#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "DS2482.h"

int main(void) {
    int file;
    char filename[40];

    sprintf(filename,"/dev/i2c-1");
    if ((file = open(filename,O_RDWR)) < 0) {
        printf("Failed to open the bus.");
        /* ERROR HANDLING; you can check errno to see what went wrong */
        exit(1);
    }

    DS2482 ds(file, 0);

    int status;

    status = ds.reset();
    printf("1-wire reset status %d\n", status);

    uint8_t addr[8];

    int count = 0;
    for (int i = 0; i < 100; i++) {
        ds.reset_search();
        while ((status = ds.search(addr)) == 1) {
            //printf("    slave: %02X-%02X%02X%02X%02X%02X%02X-%02X", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
            //printf("   calc CRC=%02X\n", ds.crc8(addr, 7));
            count++;
        }
    }

    printf("count=%d\n", count);
    printf("Last 1-wire search status: %d\n", status);

}