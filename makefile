CC= gcc
CFLAGS= -g -Wall
TARGET= sush
OBJS= sush.o modules/tokenizer.o modules/rcreader.o

all: $(TARGET)

sush: $(OBJS)
	$(CC) $(CFLAGS) -o sush $(OBJS)

run: $(TARGET)
	./sush

clean:
	rm -f *.o modules/*.o $(TARGET)
