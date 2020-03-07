CC= gcc
CFLAGS= -g -Wall
TARGETS= sush
OBJS= sush.o

all: $(TARGETS)

sush: $(OBJS)
	$(CC) $(CFLAGS) -o sush $(OBJS)

run: $(TARGETS)
	./sush

clean:
	rm -f *.o $(TARGETS)
