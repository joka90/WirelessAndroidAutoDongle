FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive 

RUN update-locale LC_ALL=C

RUN apt-get -q update \
    && apt-get -q -y install file wget cpio rsync locales liblz4-dev \
		build-essential libncurses5-dev python3-setuptools \
		git bzr cvs mercurial subversion libc6 unzip bc \
		vim llvm \
    && apt-get -q -y autoremove && apt-get -q -y clean

VOLUME /app/buildroot/dl
VOLUME /app/buildroot/output

CMD /bin/bash
