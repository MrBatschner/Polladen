FROM homeassistant/raspberrypi4-homeassistant:stable

RUN apk add --no-cache gcc libc-dev && \
        pip install --no-cache-dir RPi.GPIO && \
        apk del libc-dev gcc
