# Using _Polladen_ with Home-Assistant

That was the whole idea behind it.

# Home Assistant container image

The HomeAssistant container image (e.g. `homeassistant/raspberrypi4-homeassistant:stable`) does not include the `RPi.GPIO` Python library and installing it with `pip` requires a C toolchain. The [`Dockerfile`](./Dockerfile) takes care of that.


# Home-Assistant configuration

## Cover command_line

Home Assitant features a cover platform called `command_line` which is exactly what we need to use _Polladen_ from within Home-Assistant.

```yaml configuration.yaml
cover:
- platform: command_line
  covers:
    test_motor:
      command_open: /polladen/polladen.py --pidfile=/var/run/polladen/polladen.pid -r 1337 -c1 up
      command_close: /polladen/polladen.py --pidfile=/var/run/polladen/polladen.pid -r 1337 -c1 down
      command_stop: /polladen/polladen.py --pidfile=/var/run/polladen/polladen.pid -r 1337 -c1 stop
      unique_id: polladen_r1337_c1
      friendly_name: My Polladen Shutter
```
