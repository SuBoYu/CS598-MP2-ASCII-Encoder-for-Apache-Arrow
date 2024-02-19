FROM ubuntu:focal

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update -y -q && \
    apt-get install -y -q --no-install-recommends \
      build-essential \
      cmake \
      gdb \
      pkg-config && \
    apt-get clean && rm -rf /var/lib/apt/lists*