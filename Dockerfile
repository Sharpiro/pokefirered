FROM ubuntu:latest

WORKDIR /app

RUN apt-get update && \
    apt-get install -y build-essential gdebi-core git libpng-dev curl wget

RUN mkdir -p /usr/local/share/keyring/
RUN wget -O /usr/local/share/keyring/devkitpro-pub.gpg https://apt.devkitpro.org/devkitpro-pub.gpg
RUN echo "deb [signed-by=/usr/local/share/keyring/devkitpro-pub.gpg] https://apt.devkitpro.org stable main" > /etc/apt/sources.list.d/devkitpro.list
RUN apt-get update
RUN apt-get install -y devkitpro-pacman

RUN dkp-pacman -Sy
RUN dkp-pacman --noconfirm -S gba-dev
ENV DEVKITPRO=/opt/devkitpro
ENV DEVKITARM=${DEVKITPRO}/devkitARM
ENV DEVKITPPC=${DEVKITPRO}/devkitPPC
ENV PATH=${DEVKITPRO}/tools/bin:$PATH

RUN git clone https://github.com/pret/agbcc

WORKDIR /app/agbcc
RUN sh build.sh
RUN sh install.sh ../pokefirered 
WORKDIR /app/pokefirered

CMD make -j8
