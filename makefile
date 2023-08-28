# Uncomment lines below if you have problems with $PATH
#SHELL := /bin/bash
#PATH := /usr/local/bin:$(PATH)

all:
				platformio run

upload:
				platformio run --target upload

clean:
				platformio run --target clean

program:
				platformio run --target program

monitor:
				platformio run --target monitor

uploadfs:
				platformio run --target uploadfs

update:
				platformio update
