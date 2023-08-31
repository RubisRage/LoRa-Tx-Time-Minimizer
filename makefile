# Uncomment lines below if you have problems with $PATH
#SHELL := /bin/bash
#PATH := /usr/local/bin:$(PATH)

all:
				platformio run

compiledb:
				platformio run --target compiledb -e master

upload:
				platformio run --target upload -e master
				platformio run --target upload -e slave

updb:
				platformio run --target upload -e master_debug
				platformio run --target upload -e slave_debug

clean:
				platformio run --target clean

program:
				platformio run --target program

update:
				platformio update
