
all: dimes dimec

dimes: dimes.c dimes.h util.c util.h
	gcc -g -Wall -o dimes dimes.c util.c

dimec: dimec.c dimec.h
	gcc -g -Wall -o dimec dimec.c

clean:
	rm -f dimes dimec
