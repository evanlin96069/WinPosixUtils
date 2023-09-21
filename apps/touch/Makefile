CC = cl
CFLAGS = /W4 /GS- /O1

INCLUDE_DIR = ..\..\include
CFLAGS += /I$(INCLUDE_DIR)

DEPS = $(wildcard $(INCLUDE_DIR)/*.h)
NAME = touch
EXE = $(NAME).exe
OBJ = $(NAME).obj

.PHONY: clean

all: $(EXE)

$(EXE): touch.c $(DEPS)
	$(CC) $(CFLAGS) $<

clean:
	del $(EXE)
	del $(OBJ)
