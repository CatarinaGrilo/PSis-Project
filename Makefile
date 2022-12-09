all: chase-client


chase-client: chase-client.c chase.h
	gcc -Wall -pedantic chase-client.c -g -o chase-client -lncurses

chase-server: chase-server.c chase.h
	gcc -Wall -pedantic chase-server.c -g -o chase-server -lncurses

clean:
	rm chase-client chase-server
