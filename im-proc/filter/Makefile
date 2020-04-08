ROOT=../bcl

CPPFLAGS = -I. -I$(ROOT)/include
CFLAGS   = -Wall -Wextra -pedantic -std=c99 #-Werror
LDFLAGS  = -L$(ROOT)/lib
LDLIBS   = -lm -lbcl -lfftw3

BIN=heat-equation\
	anisotropic-diffusion\
	median\
	bilateral\
	nlmeans\
	butterworth\


.PHONY=all
all: $(BIN)

butterworth: fft.o

exec-heat:
	./heat-equation 10 ../data/barbara.ppm heat-a.ppm
	./heat-equation 50 ../data/barbara.ppm heat-b.ppm
	./heat-equation 100 ../data/barbara.ppm heat-c.ppm

exec-anistropic-diffusion:
	./anisotropic-diffusion 100 10 0 ../data/barbara.ppm anistropic-a.ppm
	./anisotropic-diffusion 100 10 1 ../data/barbara.ppm anistropic-b.ppm
	./anisotropic-diffusion 100 10 2 ../data/barbara.ppm anistropic-c.ppm

exec-median:
	./median 1 ../data/barbara-impulse.ppm median-a.ppm
	./median 2 ../data/barbara-impulse.ppm median-b.ppm
	./median 3 ../data/barbara-impulse.ppm median-c.ppm

exec-bilateral:
	./bilateral 3 30 ../data/barbara.ppm bilateral-a.ppm
	./bilateral 3 100 ../data/barbara.ppm bilateral-b.ppm
	./bilateral 10 30 ../data/barbara.ppm bilateral-c.ppm
	./bilateral 10 100 ../data/barbara.ppm bilateral-d.ppm

exec-nlmeans:
	./nlmeans 5 ../data/barbara.ppm nlmeans-a.ppm
	./nlmeans 15 ../data/barbara.ppm nlmeans-b.ppm
	./nlmeans 10 ../data/barbara-gaussian.ppm nlmeans-c.ppm

exec-butterworth:
	./butterworth ../data/lena-gray.ppm butterworth-lp.ppm lp 32 2 0 0 0
	./butterworth ../data/lena-gray.ppm butterworth-hp.ppm hp 32 2 0 0 0
	./butterworth ../data/lena-gray.ppm butterworth-br.ppm br 128 2 64 0 0
	./butterworth ../data/lena-gray.ppm butterworth-bp.ppm bp 64 2 64 0 0
	./butterworth ../data/lena-sin.ppm butterworth-no.ppm no 1 2 0 0 8


.PHONY: clean cleanall
clean:
	$(RM) *.o *.ppm
cleanall: clean
	$(RM) $(BIN)
