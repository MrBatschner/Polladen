#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "bcm2835.h"


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

typedef struct cmdline_parameters {
    int32_t remote_id;
    int8_t channel;
    char *command;
} cmdline_parameters_t;


void rf_send_bit(const uint32_t delay_sequence[]) {
    bcm2835_gpio_write(BCM_TRANSMIT_PIN, HIGH);
    bcm2835_delayMicroseconds(delay_sequence[0]);
    bcm2835_gpio_write(BCM_TRANSMIT_PIN, LOW);
    bcm2835_delayMicroseconds(delay_sequence[1]);
}

void wait_microseconds(uint64_t microseconds) {
    bcm2835_delayMicroseconds(microseconds);
}


void send_bit(const uint32_t bit) {
    if (bit) {
        rf_send_bit(delays_bit_1);
    } else {
        rf_send_bit(delays_bit_0);
    }
}

void send_datagram(transmission_t *t) {
    rf_send_bit(delays_start_of_transmission);

    for (int8_t i = 31; i >= 0; i--) {
        send_bit(t->header >> i & 1);
    }

    for (int8_t i = 7; i >= 0; i--) {
        send_bit(t->command >> i & 1);
    }
}

void send_command(transmission_t *t, uint8_t repeat) {
    uint8_t i;

    for (uint8_t i = 0; i < repeat; i++) {
        send_datagram(t);
        wait_microseconds(delay_between_datagrams);
    }

    if (t->invert) {
        t->command = (t->command & 0xf0) | ((t->command ^ 0xff) & 0x0f);
        for (uint8_t i = 0; i < repeat; i++) {
            send_datagram(t);
            wait_microseconds(delay_between_datagrams);
        }
    }
}


void print_help() {
    printf("usage: polladen [-h] [-r REMOTE_ID] [-c CHANNEL] command\n\n");
    printf("Control Dooya (and derivatives) window shutter motors over their 433MHz RF protocol.\n\n");
    printf("positional arguments:\n");
    printf("  command               the command to execute\n");
    printf("                          must be one of: up, down, stop, learn\n");
    printf("optional arguments:\n");
    printf("  -h, --help            show this help message and exit\n");
    printf("  -r REMOTE_ID, --remote-id REMOTE_ID\n");
    printf("                        the id of the remote control we are emulating\n");
    printf("  -c CHANNEL, --channel CHANNEL\n");
    printf("                        the channel to send on\n");
}

int parse_argv(int argc, char* argv[], cmdline_parameters_t *p) {
    int32_t remote_id = -1;
    int8_t channel = -1;

    static struct option long_options[] = {
        {"remote-id", required_argument, 0, 'r'},
        {"channel", required_argument, 0, 'c'},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    int c;

    while ((c = getopt_long(argc, argv, "r:c:", long_options, &option_index)) != -1) {
        switch (c) {
            case 'r':
                remote_id = atoi(optarg);
                break;
            case 'c':
                channel = atoi(optarg);
                break;
        }
    }

    if (remote_id < 1 || channel < 0 || argc - optind != 1) {
        print_help();
        return EXIT_FAILURE;
    }

    p->remote_id = remote_id;
    p->channel = channel;
    p->command = argv[optind];
}


int main(int argc, char *argv[]) {
    cmdline_parameters_t p;

    if (parse_argv(argc, argv, &p) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    transmission_t t = {
        .header = 0x1f << 24 | (p.remote_id & 0xffff) << 8 | 0xff << 4 | (p.channel & 0x0f)
    };

    if (!strcmp(p.command, "up")) {
        t.command = COMMAND_SHUTTER_UP;   
        t.invert = TRUE;
    } else if (!strcmp(p.command, "down")) {
        t.command = COMMAND_SHUTTER_DOWN;
        t.invert = TRUE;
    } else if (!strcmp(p.command, "stop")) {
        t.command = COMMAND_SHUTTER_STOP;
        t.invert = FALSE;
    } else if (!strcmp(p.command, "learn")) {
        t.command = COMMAND_SHUTTER_LEARN;
        t.invert = FALSE;
    } else {
        print_help();
        return EXIT_FAILURE;
    }

    if (!bcm2835_init()) {
        fprintf(stderr, "Failed to initialize bcm2835 library.\n");
        return EXIT_FAILURE;
    }

    bcm2835_gpio_fsel(BCM_TRANSMIT_PIN, BCM2835_GPIO_FSEL_OUTP);
    
    send_command(&t, DATAGRAM_REPEAT);

    if (!bcm2835_close()) {
        fprintf(stderr, "Failed to properly close and clean up bcm2835 library.\n");
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS; 	
}
