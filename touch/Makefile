CC = cl
CFLAGS = /W3 /GS- /O1

NAME = touch
EXE = $(NAME).exe
OBJ = $(NAME).obj

.PHONY: clean

all: $(EXE)

$(EXE): touch.c
	$(CC) $(CFLAGS) $<

clean:
	del $(EXE)
	del $(OBJ)
