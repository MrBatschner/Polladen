# Polladen

A Python tool to control a 433MHz remote controlled window shutter from an RPi.

It supports RF controlled window shutter motors from the German suppliers [3T Motors](https://www.3t-motors.de/), [Rohrmotor24](https://www.rohrmotor24.eu/rohrmotor24_1), [Julius Mayer](https://www.julius-mayer.com/julius-mayer-funk-rollladenmotor/) and possibly many many more (a good indicator is that the motor beeps when initially powered on).

The intention is to control these motors from a Raspberry Pi which has a simple 433MHz transmitter connected to one of its GPIO pins.

The name _polladen_ is for **P**ython R**olladen** (German for shutter).

## Setup

1. You need a Raspberry Pi

1. You need one of these cheap 433MHz transmitters/receivers you can find on eBay, Amazon, etc. A small PCB with one or two coils and a coin-shaped component on it.

1. Connect your 433MHz transmitter to the GPIO header of your Raspberry Pi
   These simple transmitters have three pins, `GND`, `VCC` and `ADAT`. 
   
   Connect the 433MHz transmitter to the Raspberry Pi like this:

   ```
   GND  -> Pin 6 on the GPIO header
   VCC  -> Pin 2 on the GPIO header
   ADAT -> Pin 11 on the GPIO header
   ```

1. You need the Python `RPi.GPIO` library - if not present, install it with `pip install RPi.GPIO`

1. _polladen_ emulates an RF remote, thus it needs to be paired to the motor like a normal remote.

1. Use the program and integrate it into your favorite home automation platform.

## Pairing

Every remote for these shutter motors has a unique ID that is embedded into it at the factory. When the instruction manual tells you to pair the remote with a motor (something like powering on the motor and within ten seconds press _p2-button_, _p2-button_, _up_), the motor is instructed to listen to RF signals with that remote ID in them.

The remote ID is 28 bits, allowing for 268,435,456 uniquely identifiable remotes. Polladen however sets 12 of these bits to fixed values, leaving you with 65,536 unique IDs.

When using _polladen_ you need to make up a unique remote ID (any number between `0` and `65535` - or use the default `0x42` which is `66`in decimal) that you need to supply with the `-r` parameter each time you use the program.

To pair _polladen_ to your shutter motor, follow the instruction in your motors manual. However, instead of pressing some buttons on a remote, you just take your remote ID and run the program like this:

```
./polladen.py -r <remote-id> pair
```

## Usage

```
usage: polladen.py [-h] [-r REMOTE_ID] [-c CHANNEL_ID] command

Control Dooya (and derivatives) window shutter motors over their 433MHz RF protocol.

positional arguments:
  command               the command to execute

optional arguments:
  -h, --help            show this help message and exit
  -r REMOTE_ID, --remote-id REMOTE_ID
                        the id of the remote control we are emulating
  -g GROUP, --group GROUP
                        the group to send for
```

As command, you can supply one of the following:

| Command | Purpose |
|---|---|
| `up` | Moves the shutter up |
| `down` | Moves the shutter down |
| `stop` | Stops the shutter |
| `learn` | Equivalent to the `setup` button on a remote and used to add additional remote or configure the motor |
| `pair` | Pairs _polladen_  with the given remote-id and channel-id to a motor (see your motors instruction manual) |
| `change-direction` | Changes the direction of the motor for up and down commands (see your motors instruction manual) |


## Examples

### Pairing a shutter motor to remote-id 1337 and channel 4

```
./polladen.py -r 1337 -c 4 pair
```

### Move the shutter down

```
./polladen.py -r 1337 -c 4 down
```
