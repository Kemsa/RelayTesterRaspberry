FROM debian:bullseye

ENV DEBIAN_FRONTEND=noninteractive
ENV RASPI3_BULLSEYE_TOOLCHAIN_ROOT=/opt/rpi-toolchain
ENV RASPI3_BULLSEYE_SYSROOT=/opt/rpi-sysroot
ENV PATH=${RASPI3_BULLSEYE_TOOLCHAIN_ROOT}/bin:${PATH}

RUN dpkg --add-architecture armhf \
  && apt-get update \
  && apt-get install -y --no-install-recommends \
  ca-certificates \
  cmake \
  file \
  g++-arm-linux-gnueabihf \
  gcc-arm-linux-gnueabihf \
  git \
  gdb-multiarch \
  ninja-build \
  pkg-config \
  qemu-user-static \
  qttools5-dev-tools \
  wget \
  xz-utils \
  && rm -rf /var/lib/apt/lists/*

RUN mkdir -p ${RASPI3_BULLSEYE_TOOLCHAIN_ROOT}/bin ${RASPI3_BULLSEYE_SYSROOT}
RUN ln -s /usr/bin/arm-linux-gnueabihf-gcc ${RASPI3_BULLSEYE_TOOLCHAIN_ROOT}/bin/arm-linux-gnueabihf-gcc \
  && ln -s /usr/bin/arm-linux-gnueabihf-g++ ${RASPI3_BULLSEYE_TOOLCHAIN_ROOT}/bin/arm-linux-gnueabihf-g++ \
  && ln -s ${RASPI3_BULLSEYE_SYSROOT} ${RASPI3_BULLSEYE_TOOLCHAIN_ROOT}/sysroot

RUN apt-get update \
  && apt-get install -y --no-install-recommends --download-only \
  qtbase5-dev:armhf \
  libqt5core5a:armhf \
  libqt5gui5:armhf \
  libqt5widgets5:armhf \
  libqt5dbus5:armhf \
  libqt5network5:armhf \
  libxcb1:armhf \
  libx11-6:armhf \
  libxext6:armhf \
  libxrender1:armhf \
  libxcb-xinerama0:armhf \
  && mkdir -p ${RASPI3_BULLSEYE_SYSROOT} \
  && for deb in /var/cache/apt/archives/*.deb; do dpkg-deb -x "$deb" ${RASPI3_BULLSEYE_SYSROOT}; done \
  && rm -rf /var/lib/apt/lists/* /var/cache/apt/archives/*

WORKDIR /src

CMD ["bash"]
