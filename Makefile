CC = gcc
CFLAGS = -g -c
LDFLAGS = -g -lpthread
OBJS = event_loop.o demo_app.o
EXE = exe

.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(EXE)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(EXE) *.o