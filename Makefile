CFLAGS=-g -O3 -DNDEBUG -Wall -Wextra -Wshadow -pedantic -Wno-unused-parameter -Wno-sign-compare -Werror -std=c99
LDFLAGS=

default:	boggle dict

.PHONY: TAGS clean

boggle:	boggle.o dict.o boggle-main.o
	$(CC) -o $@ $(LDFLAGS) $^

dict:	dict.o dict-main.o
	$(CC) -o $@ $(LDFLAGS) $^

clean:
	$(RM) *.o

TAGS:
	etags `find . -iname '*.c' -o -iname '*.h'`
