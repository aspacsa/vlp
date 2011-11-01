CC=gcc
vlp: vlp.o menus.o database.o utils.o reports.o
	sh -c "$(CC) -o vlp vlp.o menus.o database.o utils.o reports.o $$(mysql_config --cflags) $$(mysql_config --libs) -lform -lncurses"

vlp.o: vlp.c menus.h database.h
	$(CC) -c vlp.c

database.o: database.c database.h records.h
	sh -c "$(CC) -c database.c -std=c99 $$(mysql_config --cflags) $$(mysql_config --libs)"

menus.o: menus.c menus.h utils.h records.h reports.h
	$(CC) -c menus.c -std=c99 -lform -lncurses

reports.o: reports.c reports.h records.h
	$(CC) -c reports.c -std=c99

utils.o: utils.c utils.h
	$(CC) -c utils.c -std=c99

clean:
	rm -f *o vlp
       
