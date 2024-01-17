.PHONY: all clean

all: build_process/master build_process/server build_process/input build_process/drone build_process/target build_process/obstacle build_process/wd
	cd build_process && ./master

build_process/master: master.c | build_process
	gcc -o $@ $<

build_process/server: server.c | build_process
	gcc -o $@ $< -lncurses -lm

build_process/input: input.c | build_process
	gcc -o $@ $< -lncurses

build_process/drone: drone.c | build_process
	gcc -o $@ $< -lncurses

build_process/target: target.c | build_process
	gcc -o $@ $< -lncurses

build_process/obstacle: obstacle.c | build_process
	gcc -o $@ $< -lncurses

build_process/wd: wd.c | build_process
	gcc -o $@ $<

build_process:
	mkdir -p build_process

clean:
	rm -rf build_process
