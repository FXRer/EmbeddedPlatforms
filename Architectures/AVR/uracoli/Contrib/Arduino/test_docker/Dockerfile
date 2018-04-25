#
# This container provides a framework for the arduino development
# tool chain.
# The application itself is mapped with the "-v" option in the
# container. So it is easily possible to test different IDE-versions
# with this container.
#
# Usage:
#  scons  parduino15  url=http://172.17.0.1:8000
#
#  docker run -it --env DISPLAY=:0    \
#         -v /tmp/.X11-unix:/tmp/.X11-unix \
#         -v $(pwd)/arduino-1.6.10:/opt/myarduino \
#         -v $(pwd)/arduino_settings:/home/uracoli/.arduino15 \
#         --device=/dev/ttyUSB0 \
#         arduino
#
FROM ubuntu

MAINTAINER Axel Wachtler axel@uracoli.de

# install packages
RUN apt-get update \
    && apt-get -y install \
        arduino \
        curl \
        iputils-ping \
        nano \
        net-tools \
        python \
        x11vnc \
        xvfb \
        xz-utils

# add the development user and create a working directory

ENV USER=arduino
#ENV ARDUINO_ZIP=arduino-1.6.10-linux64.tar.xz
#ENV ARDUINO_ZIP=arduino-1.8.2-linux64.tar.xz

RUN useradd -ms /bin/bash ${USER}
RUN usermod -a -G dialout ${USER}

RUN mkdir /home/${USER}/.arduino15
RUN mkdir /home/${USER}/Arduino
RUN mkdir /home/${USER}/Arduino/libraries
RUN touch /home/${USER}/Arduino/libraries/readme.txt
RUN mkdir /home/${USER}/uracoli

ADD uracoli/package_uracoli_index.json /home/${USER}/uracoli
ADD preferences.txt /home/${USER}/.arduino15
ADD run.sh /home/${USER}

# get https://downloads.arduino.cc/arduino-1.8.2-linux64.tar.xz /opt/
# local versions for comparission.

# use this spell to extract the arduino archive right into the image.
# "ADD https://.../arduino-1.8.2-linux64.tar.xz /opt" fails
RUN curl https://downloads.arduino.cc/arduino-1.8.2-linux64.tar.xz \
   | tar -xJC /opt/
#ADD arduino-1.6.10-linux64.tar.xz /opt
#ADD arduino-1.6.13-linux64.tar.xz /opt
#ADD arduino-1.8.2-linux64.tar.xz /opt

# VNC

RUN mkdir -p /home/${USER}/.vnc
RUN x11vnc -storepasswd 1234 /home/${USER}/.vnc/passwd
EXPOSE 5900
# reown contents of home dir
RUN chown -R ${USER}.${USER} /home/${USER}

WORKDIR /home/${USER}
USER  ${USER}

# This commands are running at the start of the container
CMD \
    echo == starting arduino environment == \
    && bash run.sh \
    && /bin/bash
