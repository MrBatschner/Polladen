#!/usr/bin/env python3

# Lots of information on the inner workings of this protocol can be found at https://www.mikrocontroller.net/topic/242784
# Timimgs for the protocol were re-measured using a scope and an HS1 handheld remote from 3T-Motors.
# This code should accurately emulate said remote.

import argparse
import sys
import time
import RPi.GPIO as GPIO


# timing constants
delays_command_bit_0   = [320, 620]
delays_command_bit_1   = [620, 320]
delays_sync            = [4500, 1400]
delay_between_sequence = 7400
delay_between_presses  = 20000

# Raspberry Pi GPIO settings
rf_transmit_pin       = 17

# command identifiers
command_shutter_up    = 0x11    # 0001 0001 
command_shutter_down  = 0x33    # 0011 0011
command_shutter_stop  = 0x55    # 0101 0101 
command_learn         = 0xcc    # 1100 1100 


def delay(microseconds: int):
    time.sleep(microseconds / 1000000)


def send_bit(delay_sequence: list[int]):
    GPIO.output(rf_transmit_pin, True)
    delay(delay_sequence[0])
    GPIO.output(rf_transmit_pin, False)
    delay(delay_sequence[1])


def send_sequence(sequence: int):
    send_bit(delays_sync)  # start of transmission - sync

    # actual code
    for offset in range(40):
        mask = 1 << (40 - 1 - offset)
        if sequence & mask == 0:
            send_bit(delays_command_bit_0)
        else:
            send_bit(delays_command_bit_1)


def send_command(header: int, command: int, repeat: int = 6, invert: bool = True):
    sequence = header | command

    for x in range(repeat):
        send_sequence(sequence=sequence)
        delay(delay_between_sequence)
    
    if invert:
        sequence = header | (command & 0xf0) | ((command ^ 0xff) & 0x0f)
        for x in range(repeat):
            send_sequence(sequence=sequence)
            delay(delay_between_sequence)


def up(header: int):
    send_command(header=header, command=command_shutter_up)
    

def down(header: int):
    send_command(header=header, command=command_shutter_down)


def stop(header: int):
    send_command(header=header, command=command_shutter_stop, invert=False)


def learn(header: int):
    send_command(header=header, command=command_learn, invert=False)

def p2(header: int):
    learn(header=header)


def pair(header: int):
    learn(header)
    delay(delay_between_presses)
    learn(header)
    delay(5 * delay_between_presses)
    up(header)


def change_direction(header: int):
    learn(header)
    delay(delay_between_presses)
    learn(header)
    delay(5 * delay_between_presses)
    down(header)


def setup_and_run_argparser():
    parser = argparse.ArgumentParser(description='Control Dooya (and derivatives) window shutter motors over their 433MHz RF protocol.')
    parser.add_argument('command', metavar='command', type=str, nargs=1, help='the command to execute', choices=['up', 'down', 'stop', 'learn', 'p2', 'pair', 'change_direction'])
    parser.add_argument('-r', '--remote-id', dest="remote_id", type=int, default=0x42, help='the id of the remote control we are emulating')
    parser.add_argument('-c', '--channel', dest="channel", type=int, default=1, help='the channel to send on')
    return parser.parse_args()


def setup_raspberry_pi_gpio():
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(rf_transmit_pin, GPIO.OUT)
    GPIO.output(rf_transmit_pin, False)


if __name__ == '__main__':
    args = setup_and_run_argparser()
    setup_raspberry_pi_gpio()

    header = 0x1f << 32 | (args.remote_id & 0xffff) << 16 | 0xff << 12 | (args.channel & 0x0f) << 8
    
    module_symbols = sys.modules[__name__]
    func = getattr(module_symbols, args.command[0], None) 
    func(header=header)

    GPIO.cleanup()
