#
#
# Makefile
#
# client.c


nix:
	gcc -ggdb -std=c99 -Wall -Werror -Wformat=0 -o client client.c -lncurses



windows:
	gcc -o client.o -c client.c -I /usr/include/ncurses
	gcc -o client.exe client.o /usr/lib/libncurses.dll.a




clean:
	rm -f core client.exe *.o client

