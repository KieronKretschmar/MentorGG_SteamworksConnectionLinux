FROM ubuntu:precise as build
MAINTAINER Arran Smith <arran@mentor.gg>

RUN apt-get update

RUN apt-get install -y \

    unzip \
    autoconf \
    automake \
    libtool \
    make \
    g++ \
    curl



# region Protobuf 2.5.0 Installation
WORKDIR /opt/
RUN curl -L https://github.com/protocolbuffers/protobuf/releases/download/v2.5.0/protobuf-2.5.0.zip --output protobuf.zip
RUN unzip protobuf.zip

WORKDIR /opt/protobuf-2.5.0
RUN ./configure
RUN make
RUN make check
RUN make install
RUN ldconfig
# endregion 


COPY ./ /app
WORKDIR /app

RUN ./makeprotos.sh

# Build
RUN g++ -std=c++0x src/*.cpp proto_build/*.cc -Lsteamworks_sdk/redistributable_bin/linux64/ -lsteam_api -Lprotobuf -lprotobuf -I. -Iproto_build -o swc -pthread





