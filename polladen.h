#ifndef __POLLADEN_H__
#define __POLLADEN_H__

#include <stdint.h>

/* definition of the command identifiers */
#define COMMAND_SHUTTER_UP      0x11
#define COMMAND_SHUTTER_DOWN    0x33
#define COMMAND_SHUTTER_STOP    0x55
#define COMMAND_SHUTTER_LEARN   0xcc

#define DATAGRAM_REPEAT         5

/* the Raspberry Pi GPIO pin the RF transmitter is connected to */
#define BCM_TRANSMIT_PIN        17

/* I like boolean names */
#define BOOLEAN uint8_t
#define TRUE    1
#define FALSE   0

/* timings for the 433MHz RF transmitter in microseconds */
const uint32_t delays_bit_0[] = {320, 620};
const uint32_t delays_bit_1[] = {620, 320};
const uint32_t delays_start_of_transmission[] = {4500, 1400};

const uint32_t delay_between_datagrams = 7400;
const uint32_t delay_between_transmissions = 20000;

typedef struct transmission {
    uint32_t header;
    uint8_t command;
    BOOLEAN invert;
} transmission_t;


void wait_microseconds(uint64_t microseconds);

void rf_send_bit(const uint32_t delay_sequence[]);
void send_bit(const uint32_t bit);
void send_datagram(transmission_t *t);
void send_command(transmission_t *t, uint8_t repeat);

#endif
