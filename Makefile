INCLUDES += -I$(CURDIR)/using/tinycthread/source/

all: *.c using/tinycthread/source/tinycthread.c
	gcc -Wall -g $(INCLUDES) *.c using/tinycthread/source/tinycthread.c -lpthread