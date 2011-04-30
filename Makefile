all: dimes dimec

dimes: dimes.c dimes.h util.c util.h
	gcc -g -Wall -o dimes dimes.c util.c -lpthread

dimec: dimec.c dimec.h util.c util.h
	gcc -g -Wall -o dimec dimec.c util.c -lpthread

clean:
	rm -f dimes dimec
