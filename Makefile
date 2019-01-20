.PHONY =  all
CFLAGS =  -g

all: deescheme
deescheme: deescheme.c deescheme.h
	gcc $(CFLAGS) -o $@ $<
