ROOT=../bcl

CPPFLAGS = -I$(ROOT)/include -I.
CFLAGS   = -Wall -Wextra -pedantic -std=c99 --debug #-Werror
LDFLAGS  = -L$(ROOT)/lib
LDLIBS   = -lbcl -lm

BIN=colorization

.PHONY: all
all: $(BIN)

.PHONY: clean cleanall
clean:
	$(RM) *.o *.ppm
cleanall: clean
	$(RM) $(BIN)
