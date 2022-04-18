clean:
	rm -rf out/* scheduler

all: clean
	gcc scheduler.c queue.c -o scheduler
