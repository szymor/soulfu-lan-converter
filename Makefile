.PHONY: all clean

PROJECT=slc
SRC=main.c

all: $(PROJECT)

$(PROJECT): $(SRC)
	gcc -o $(PROJECT) $(SRC)

clean:
	-rm -f $(PROJECT)
