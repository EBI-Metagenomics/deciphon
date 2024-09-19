SIMD =
ifeq ($(shell uname -p),x86_64)
	SIMD += -mavx2
endif

OMP =
ifeq ($(shell uname),Linux)
	OMP += -fopenmp
endif

CC     ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -O3 -MMD -MP $(SIMD) $(OMP)
PREFIX ?= /usr/local
SRC = $(filter-out $(wildcard test_*.c),$(wildcard *.c))
OBJ = $(SRC:.c=.o)
HDR = deciphon.h
LIB = libdeciphon.a
TEST_SRC = $(wildcard test_*.c)
TEST_OBJ = $(TEST_SRC:.c=.o)
TEST_TARGET = $(basename $(TEST_OBJ))


all: $(LIB)

$(LIB): $(OBJ)
	ar rcs $@ $^

-include $(SRC:.c=.d)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_TARGET): %: %.o $(LIB)
	$(CC) $(CFLAGS) $< -L. -ldeciphon -lh3client -lh3result -lhmmer_reader -limm -llio -llite_pack -lm -o $@

check: $(TEST_TARGET)
	./test_match
	./test_model
	./test_press
	./test_protein
	./start_daemon.sh massive.hmm 51300 && ./test_massive; e=$$?; ./stop_daemon.sh massive.hmm; exit $$e
	./start_daemon.sh minifam.hmm 51300 && ./test_scan   ; e=$$?; ./stop_daemon.sh minifam.hmm; exit $$e
	./start_daemon.sh minifam.hmm 51300 && ./test_window ; e=$$?; ./stop_daemon.sh minifam.hmm; exit $$e

install: $(LIB) $(HDR)
	@mkdir -p $(PREFIX)/lib $(PREFIX)/include
	install -m 0755 $(LIB) $(PREFIX)/lib/
	install -m 0644 $(HDR) $(PREFIX)/include/

uninstall:
	rm -f $(PREFIX)/lib/$(LIB) $(HDR:%=$(PREFIX)/include/%)

.PHONY: all clean check uninstall
clean:
	rm -f $(OBJ) $(LIB) $(TEST_OBJ) $(TEST_TARGET) *.d
