URACOLI_INSTALL=$(abspath ../../../install)
SERIAL=/dev/ttyUSB0

ARDUINO=~/Downloads/arduino-1.6.10-linux64.tar.xz
#ARDUINO=~/Downloads/arduino-1.8.2-linux64.tar.xz
ARDUINO_ZIP=$(notdir $(ARDUINO))

DOCKER_FILE=Dockerfile
DOCKER_IMAGE=arduino_testenv
DOCKER_OPTS=-it \
            -e DISPLAY=:0 -p 5900\
            -v /tmp/.X11-unix:/tmp/.X11-unix \
            -v $(shell pwd)/uracoli:/home/arduino/uracolixxx\
            -v $(URACOLI_INSTALL):/home/arduino/uracoli\

DOCKER_SERIAL=$(DOCKER_OPTS) --device=$(SERIAL)
DOCKER_NOSERIAL=$(DOCKER_OPTS)



.PHONY: all prepare build run clean

all: prepare build run

prepare: uracoli
	cp -rfv $(URACOLI_INSTALL)/package_uracoli_index.json \
		$(URACOLI_INSTALL)/uracoli-arduino-15x-*.zip \
		uracoli

build:
	docker build  -f $(DOCKER_FILE) -t $(DOCKER_IMAGE) .

run:
	docker run $(DOCKER_OPTS) $(DOCKER_IMAGE)
run1:
	docker run $(DOCKER_NOSERIAL) $(DOCKER_IMAGE)

clean:
	-docker rm $(docker ps -aq)
	-docker rmi $(docker images -f "dangling=true" -q)


uracoli:
	test -d uracoli || mkdir -p uracoli
