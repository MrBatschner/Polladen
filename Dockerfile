####### Polladen build stage

FROM alpine:latest as polladen-builder

RUN apk add --no-cache libc-dev gcc make curl

RUN curl -O http://www.airspayce.com/mikem/bcm2835/bcm2835-1.71.tar.gz \
    && tar -xzf bcm2835-1.71.tar.gz \
    && cd bcm2835-1.71  \
    && ./configure \
    && make

COPY polladen.* /polladen/

WORKDIR /polladen

RUN gcc -c -I/bcm2835-1.71/src -o polladen.o polladen.c \
    && gcc -o polladen polladen.o /bcm2835-1.71/src/libbcm2835.a \
    && strip polladen


####### Polladen container

FROM alpine:latest

COPY --from=polladen-builder /polladen/polladen /usr/bin/polladen
