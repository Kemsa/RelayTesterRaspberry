FROM --platform=linux/arm/v7 debian:bookworm

ENV DEBIAN_FRONTEND=noninteractive
ENV RASPI3_BOOKWORM_TOOLCHAIN_ROOT=/opt/rpi-toolchain
ENV RASPI3_BOOKWORM_SYSROOT=/opt/rpi-sysroot
ENV PATH=${RASPI3_BOOKWORM_TOOLCHAIN_ROOT}/bin:${PATH}

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
  build-essential \
  gnupg2 \
  && rm -rf /var/lib/apt/lists/*

RUN mkdir -p ${RASPI3_BOOKWORM_TOOLCHAIN_ROOT}/bin ${RASPI3_BOOKWORM_SYSROOT} \
  && ln -s /usr/bin/arm-linux-gnueabihf-gcc ${RASPI3_BOOKWORM_TOOLCHAIN_ROOT}/bin/arm-linux-gnueabihf-gcc \
  && ln -s /usr/bin/arm-linux-gnueabihf-g++ ${RASPI3_BOOKWORM_TOOLCHAIN_ROOT}/bin/arm-linux-gnueabihf-g++ \
  && ln -s ${RASPI3_BOOKWORM_SYSROOT} ${RASPI3_BOOKWORM_TOOLCHAIN_ROOT}/sysroot

RUN apt-get update \
  && apt-get install -y --no-install-recommends --download-only --reinstall \
  libc6:armhf \
  libc6-dev:armhf \
  linux-libc-dev:armhf \
  zlib1g:armhf \
  libdouble-conversion3:armhf \
  libicu72:armhf \
  libpcre2-16-0:armhf \
  libpcre2-8-0:armhf \
  libzstd1:armhf \
  libglib2.0-0:armhf \
  libusb-1.0-0:armhf \
  libudev1:armhf \
  libgles2:armhf \
  libglvnd0:armhf \
  libpng16-16:armhf \
  libharfbuzz0b:armhf \
  libfreetype6:armhf \
  libbrotli1:armhf \
  libgraphite2-3:armhf \
  libmd4c0:armhf \
  qtbase5-dev:armhf \
  libqt5core5a:armhf \
  libqt5gui5:armhf \
  libqt5widgets5:armhf \
  libqt5serialport5:armhf \
  libqt5serialport5-dev:armhf \
  libqt5dbus5:armhf \
  libqt5network5:armhf \
  libxcb1:armhf \
  libx11-6:armhf \
  libxext6:armhf \
  libxrender1:armhf \
  libxcb-xinerama0:armhf \
  && mkdir -p ${RASPI3_BOOKWORM_SYSROOT} \
  && for deb in /var/cache/apt/archives/*.deb; do dpkg-deb -x "$deb" ${RASPI3_BOOKWORM_SYSROOT}; done \
  && rm -rf /var/lib/apt/lists/* /var/cache/apt/archives/*

RUN bash -c 'wget -O- https://labs.picotech.com/Release.gpg.key | gpg --dearmor > /usr/share/keyrings/picotech-archive-keyring.gpg' \
  && bash -c 'echo "deb [signed-by=/usr/share/keyrings/picotech-archive-keyring.gpg] https://labs.picotech.com/picoscope7/debian/ picoscope main" >/etc/apt/sources.list.d/picoscope7.list' \
  && apt-get update \
  && apt-get install -y --no-install-recommends libpicohrdl

RUN wget https://github.com/WiringPi/WiringPi/releases/download/3.18/wiringpi_3.18_armhf.deb \
  && dpkg -i wiringpi_3.18_armhf.deb

WORKDIR /src

CMD ["bash"]
