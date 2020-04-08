ROOT=../bcl

CPPFLAGS = -I. -I$(ROOT)/include
CFLAGS   = -Wall -Wextra -Werror -pedantic -std=c99
LDFLAGS  = -L$(ROOT)/lib
LDLIBS   = -lm -lbcl

BIN=heat-equation\
	anisotropic-diffusion\
	median\
	bilateral\
	nlmeans\
	#butterworth\


.PHONY=all
all: $(BIN)

#butterworth: fft.o

test-median:
	./median 1 ../data/barbara-impulse.ppm a.ppm
	./median 2 ../data/barbara-impulse.ppm a.ppm
	./median 3 ../data/barbara-impulse.ppm a.ppm

.PHONY: clean cleanall
clean:
	$(RM) *.o *.ppm
cleanall: clean
	$(RM) $(BIN)
