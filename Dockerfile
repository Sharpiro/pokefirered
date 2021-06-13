FROM ubuntu:latest

WORKDIR /app

RUN apt-get update
RUN apt-get install -y build-essential gdebi-core git libpng-dev curl

RUN curl -sLO https://github.com/devkitPro/pacman/releases/latest/download/devkitpro-pacman.amd64.deb
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends tzdata

ENV DEVKITPRO=/opt/devkitpro
ENV DEVKITARM=$DEVKITPRO/devkitARM
RUN gdebi -n devkitpro-pacman.amd64.deb
RUN dkp-pacman -Sy
RUN dkp-pacman --noconfirm -S gba-dev
RUN . /etc/profile.d/devkit-env.sh

RUN git clone https://github.com/pret/agbcc

WORKDIR /app/agbcc
RUN sh build.sh

CMD sh install.sh ../pokefirered && cd ../pokefirered && make
