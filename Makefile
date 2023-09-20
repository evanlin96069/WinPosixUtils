CC = clang
CFLAGS = -Wall -Wextra -pedantic -std=c99 -O2

EXE = touch.exe

.PHONY: clean

all: $(EXE)

$(EXE): src/touch.c src/wopt.h
	$(CC) -o $@ $<

clean:
	del $(EXE)
