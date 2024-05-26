
CC=gcc
CFLAGS=-Wall -Wextra -fPIC -m32
LDFLAGS=

OBJS=obj/main.c.o
BIN=libnetvardumper.so

.PHONY: clean all inject

# -------------------------------------------

all: $(BIN)

clean:
	rm -f $(OBJS)
	rm -f $(BIN)

inject: $(BIN)
	bash ./inject.sh

# -------------------------------------------

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -shared -o $@ $(OBJS) $(LDFLAGS)

$(OBJS): obj/%.c.o : src/%.c
	@mkdir -p obj/
	$(CC) $(CFLAGS) -c -o $@ $< $(LDFLAGS)
