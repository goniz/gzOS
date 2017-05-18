FROM ubuntu:16.04

RUN apt-get update && \
    apt-get -y dist-upgrade && \
    echo 'Installing native toolchain and build system functionality' >&2 && \
    apt-get -y install \
        build-essential \
        cmake \
        wget && \
    echo 'Cleaning up installation files' >&2 && \
    apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# Install MIPS binary toolchain
RUN mkdir -p /opt && \
        wget -q http://codescape-mips-sdk.imgtec.com/components/toolchain/2016.05-03/Codescape.GNU.Tools.Package.2016.05-03.for.MIPS.MTI.Bare.Metal.CentOS-5.x86_64.tar.gz -O- \
        | tar -C /opt -xz

ENV PATH $PATH:/opt/mips-mti-elf/2016.05-03/bin
ENV MIPS_ELF_ROOT /opt/mips-mti-elf/2016.05-03

COPY . /code
WORKDIR /code
RUN make clean malta
RUN make
