all: chase-client chase-server chase-bot


chase-client: ./Client/chase-client.c chase.h
	gcc -Wall -pedantic ./Client/chase-client.c -g -o chase-client -lncurses

chase-server: ./Server/chase-server.c chase.h
	gcc -Wall -pedantic ./Server/chase-server.c -g -o chase-server -lncurses

chase-bot: ./Bot/chase-bot.c chase.h
	gcc -Wall -pedantic ./Bot/chase-bot.c -g -o chase-bot -lncurses

clean:
	rm chase-client chase-server chase-bot
