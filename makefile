CC= gcc
CFLAGS= -g -Wall
TARGET= sush
OBJS= sush.o tokenizer.o rcreader.o

all: $(TARGET)

sush: $(OBJS)
	$(CC) $(CFLAGS) -o sush $(OBJS)

run: $(TARGET)
	./sush

clean:
	rm -f *.o $(TARGET)
