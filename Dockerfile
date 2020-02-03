# =================
# BUILD IMAGE
# =================

FROM ubuntu:xenial as build
MAINTAINER Arran Smith <arran@mentor.gg>

# Install Requirements for build

RUN apt-get update

RUN apt-get install -y \

    unzip \
    autoconf \
    automake \
    libtool \
    make \
    g++ \
    curl


#  Protobuf 2.5.0 Installation
WORKDIR /opt/
RUN curl -L https://github.com/protocolbuffers/protobuf/releases/download/v2.5.0/protobuf-2.5.0.zip --output protobuf.zip
RUN unzip protobuf.zip

WORKDIR /opt/protobuf-2.5.0
RUN ./configure
RUN make
RUN make check
RUN make install
RUN ldconfig

# Copy the repo to /app

COPY ./ /app
WORKDIR /app

# Compile the protobufs
RUN ./makeprotos.sh

# Build SteamWorksConnection (swc)
RUN mkdir out
RUN g++ src/*.cpp proto_build/*.cc -Lsteamworks_sdk/redistributable_bin/linux64/ -lsteam_api -Lprotobuf -lprotobuf -I. -Iproto_build -o out/swc -pthread -std=c++11

# Install steamworks SDK library
RUN mkdir -p /usr/lib/steamapi/
COPY steamworks_sdk/redistributable_bin/linux64/libsteam_api.so /usr/lib/steamapi/
RUN ldconfig

# Install Steam
RUN apt-get install lib32gcc1 -y
RUN mkdir /opt/Steam
WORKDIR /opt/Steam
RUN curl -sqL "https://steamcdn-a.akamaihd.net/client/installer/steamcmd_linux.tar.gz" | tar zxvf -
RUN ls -l

# Debugging
RUN apt-get install vim -y


ENV LD_LIBRARY_PATH=/usr/lib/steamapi
ENTRYPOINT ["bash", "/app/out/swc"]



