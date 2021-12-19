CFLAGS =  -O2 -ftrapv -fsanitize=undefined -Wall -Werror -Wformat-security -Wignored-qualifiers -Winit-self -Wswitch-default -Wfloat-equal -Wshadow -Wpointer-arith -Wtype-limits -Wempty-body  -Wmissing-field-initializers -Wcast-qual -Wwrite-strings

all: client server clean_comp




table.o: table.c table.h
	gcc $(CFLAGS) -c $< -o $@

TableWrap.o: TableWrap.cpp TableWrap.hpp table.o
	g++ $(CFLAGS) -c $< -o $@

socket_wrap.o: socket_wrap.cpp socket_wrap.hpp
	g++ $(CFLAGS) -c $< -o $@

client: client.cpp socket_wrap.o
	g++ $(CFLAGS) $^ -o $@

server: server.cpp socket_wrap.o Parser.o Scanner.o Poliz.o TableWrap.o table.o
	g++ $(CFLAGS) $^ -o $@

Scanner.o: Scanner.cpp Scanner.hpp
	g++ $(CFLAGS) -c $< -o $@

Parser.o: Parser.cpp Parser.hpp Scanner.o
	g++ $(CFLAGS) -c $< -o $@

Poliz.o: Poliz.cpp Parser.o Scanner.o TableWrap.o
	g++ $(CFLAGS) -c $< -o $@

clean:
	-rm -rf *.o
	-rm -rf client
	-rm -rf server
	-rm -rf *.dSYM

clean_comp:
	-rm -rf *.o
	-rm -rf *.dSYM

