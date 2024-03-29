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

FROM ubuntu:20.04 as picotool-build-stage

ARG PICOTOOL_VERSION=1.1.1
ARG PICOTOOL_SDK_VERSION=1.5.0

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    git \
    build-essential \
    cmake \
    libusb-1.0-0-dev \
    pkg-config

RUN git clone https://github.com/raspberrypi/pico-sdk.git -b ${PICOTOOL_SDK_VERSION}

RUN git clone https://github.com/raspberrypi/picotool.git -b ${PICOTOOL_VERSION} && \
    mkdir build && \
    cd build && \
    PICO_SDK_PATH=/pico-sdk cmake /picotool && \
    make && \
    make install

FROM ubuntu:20.04 as cmake-build-stage

ARG CMAKE_VERSION=3.25
ARG CMAKE_BUILD=1
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    build-essential \
    libtool \
    autoconf \
    libssl-dev

ADD https://cmake.org/files/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}.${CMAKE_BUILD}.tar.gz /

RUN tar xf cmake-${CMAKE_VERSION}.${CMAKE_BUILD}.tar.gz

WORKDIR /cmake-${CMAKE_VERSION}.${CMAKE_BUILD}

RUN ./bootstrap && \
    make && \
    make install

FROM ubuntu:20.04

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    make \
    git \
    python3 \
    libusb-1.0-0-dev \
    texinfo \
    libhidapi-dev \
    libncurses5 \
    python3-venv \
    python3-pip \
    minicom

RUN useradd --create-home builder -s /bin/bash
RUN adduser builder plugdev
RUN adduser builder dialout

# add openocd to the docker image
COPY --from=openocd-build-stage /usr/local/bin /usr/local/bin
COPY --from=openocd-build-stage /usr/local/share /usr/local/share

COPY --from=picotool-build-stage /usr/local/bin /usr/local/bin

COPY --from=cmake-build-stage /usr/local/bin /usr/local/bin
COPY --from=cmake-build-stage /usr/local/share /usr/local/share