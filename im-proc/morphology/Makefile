ROOT=../bcl

CPPFLAGS = -I$(ROOT)/include -I.
CFLAGS   = -Wall -Wextra -pedantic -std=c99 #-Werror
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
	binarization\

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
	./opening 2 20 ../data/gear.ppm extract-disk.ppm
	./substract ../data/gear.ppm  extract-disk.ppm extract-gear.ppm

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
	./make-se 1 20 se-1.ppm
	./make-se 2 20 se-2.ppm
	./make-se 3 10 se-3.ppm
	./make-se 4 10 se-4.ppm
	./make-se 5 10 se-5.ppm
	./make-se 6 10 se-6.ppm
	./make-se 7 10 se-7.ppm
	./make-se 8 10 se-8.ppm

de:
	./dilation 2 10 ../data/lena-gray.ppm dilation-disk.ppm
	./dilation 7 10 ../data/lena-gray.ppm dilation-cross.ppm
	./erosion 2 10 ../data/lena-gray.ppm erosion-disk.ppm
	./erosion 7 10 ../data/lena-gray.ppm erosion-cross.ppm

oc:
	./opening 2 10 ../data/lena-gray.ppm opening-disk.ppm
	./opening 7 10 ../data/lena-gray.ppm opening-cross.ppm
	./closing 2 10 ../data/lena-gray.ppm closing-disk.ppm
	./closing 7 10 ../data/lena-gray.ppm closing-cross.ppm

.PHONY: clean cleanall
clean:
	$(RM) *.o *.ppm
cleanall: clean
	$(RM) $(BIN)
