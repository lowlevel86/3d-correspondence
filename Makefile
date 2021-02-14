CC = gcc
RES = 
OBJ = main.o avecis.o correspond3d.o drawPtElement.o ucrot.o $(RES)
LIBS = -mwindows -lws2_32
BIN = prog.exe
CFLAGS = -Wall
RM = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after


clean: clean-custom
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $(BIN) $(LIBS)

main.o: main.c
	$(CC) -c main.c -o main.o $(CFLAGS)

avecis.o: avecis.c
	$(CC) -c avecis.c -o avecis.o $(CFLAGS)

correspond3d.o: correspond3d.c
	$(CC) -c correspond3d.c -o correspond3d.o $(CFLAGS)

drawPtElement.o: drawPtElement.c
	$(CC) -c drawPtElement.c -o drawPtElement.o $(CFLAGS)

ucrot.o: ucrot.c
	$(CC) -c ucrot.c -o ucrot.o $(CFLAGS)
