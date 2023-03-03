FROM ubuntu:20.04 as openocd-build-stage

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    git \
    make \
    libusb-1.0-0-dev \
    libtool \
    pkg-config \
    autoconf \
    automake \
    texinfo \
    libhidapi-dev

RUN git clone https://github.com/openocd-org/openocd.git -b v0.12.0 && \
    cd openocd && \
    ./bootstrap && \
    ./configure && \
    make && \
    make install

FROM ubuntu:20.04

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    make \
    cmake \
    git \
    python3 \
    build-essential \
    libusb-1.0-0-dev \
    texinfo \
    libhidapi-dev \
    libncurses5 \
    python3-venv \
    python3-pip \
    minicom

RUN useradd --create-home builder -s /bin/bash

# add openocd to the docker image
COPY --from=openocd-build-stage /usr/local/bin/openocd /usr/local/bin/openocd
COPY --from=openocd-build-stage /usr/local/share/openocd /usr/local/share/openocd

RUN adduser builder plugdev
RUN adduser builder dialout
