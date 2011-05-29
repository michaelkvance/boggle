CFLAGS=-g -Wall -Wshadow -Werror -std=c99
LDFLAGS=

default:	bog search dict

.PHONY: TAGS clean

bog:	boggle.o main.o
	$(CC) -o $@ $(LDFLAGS) $^

search:	search.o
	$(CC) -o $@ $(LDFLAGS) $^

dict:	dict.o
	$(CC) -o $@ $(LDFLAGS) $^

clean:
	$(RM) *.o

TAGS:
	etags `find . -iname '*.c' -o -iname '*.h'`
