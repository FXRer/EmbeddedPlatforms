#
# This container provides the µracoli development tool chain.
#

FROM ubuntu

MAINTAINER Axel Wachtler axel@uracoli.de

# install packages
RUN apt-get update \
&& apt-get -y install \
    # build system related
    scons mercurial \
    # Python stuff
    python python-pip \
    # AVR Tools
    binutils-avr gcc-avr avr-libc \
    # Documentation tools
    doxygen
    # we skip doxygen-latex here, otherwise the image would become "huuuuge"
    # doxygen-latex

# add the development user and create a working directory
RUN useradd -ms /bin/bash uracoli
USER  uracoli
RUN mkdir /home/uracoli/work
WORKDIR /home/uracoli/work

# This commands are running at the start of the container
CMD \
    echo == starting uracoli-development environment == \
    && /bin/bash
