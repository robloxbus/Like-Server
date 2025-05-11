CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -g

all: parent primary likes

parent: parent.o

primary: primary.o

likes: likes.o

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%: %.o
	$(CC) $(LDFLAGS) $< -o $@


clean:
	rm -f *.o parent primary likes
