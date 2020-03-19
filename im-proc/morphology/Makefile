ROOT=../bcl

CPPFLAGS = -I$(ROOT)/include -I.
CFLAGS   = -Wall -Wextra -Werror -pedantic -std=c99
LDFLAGS  = -L$(ROOT)/lib
LDLIBS   = -lbcl -lm

VIEWER = pvisu
DATA   = ../data

BIN=\
	make-se\
	labeling\
	dilation\
	erosion\
	opening\
	closing\
	add\
	substract\

.PHONY: all
all: $(OBJ) $(BIN)

morphology.o: se.o
make-se: se.o
dilation: se.o morphology.o
erosion: se.o morphology.o
opening: se.o morphology.o
closing: se.o morphology.o
add:
substract:

.PHONY: extract-gear
extract-gear:
	$(VIEWER) $(DATA)/gear.ppm

.PHONY: morphology-mrg morphology-lex morphology-bit
morphology-mrg: se.o
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o morphology.o $@.c
	make test-color
morphology-lex: se.o
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o morphology.o $@.c
	make test-color
morphology-bit: se.o
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o morphology.o $@.c
	make test-color

.PHONY: test-color
test-color: all
	./dilation 2 1 $(DATA)/mm-color.ppm a.ppm; #$(VIEWER) a.ppm
	./erosion 2 1 $(DATA)/mm-color.ppm b.ppm; #$(VIEWER) b.ppm

se:
	./make-se 0 10 se-0.ppm
	./make-se 1 15 se-1.ppm
	./make-se 2 20 se-2.ppm
	./make-se 3 10 se-3.ppm
	./make-se 4 10 se-4.ppm
	./make-se 5 10 se-5.ppm
	./make-se 6 10 se-6.ppm
	./make-se 7 10 se-7.ppm
	./make-se 8 10 se-8.ppm

.PHONY: clean cleanall
clean:
	$(RM) *.o *.ppm
cleanall: clean
	$(RM) $(BIN)
