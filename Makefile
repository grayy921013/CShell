CC=gcc
CFLAGS=-Wall

EXE=main
OBJ=main.o shell.o invoker.o builtInCmds.o

all: ${EXE}

main: ${OBJ}

clean:
	rm -f ${OBJ} ${EXE}
