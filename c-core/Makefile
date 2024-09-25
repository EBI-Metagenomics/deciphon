SIMD ?=
ifeq ($(shell uname -m),x86_64)
	SIMD += -mavx2
endif

OMP ?=
ifeq ($(shell uname),Linux)
	OMP += -fopenmp
endif

CC     ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -O3 -MMD -MP
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
	$(CC) $(CFLAGS) $(SIMD) $(OMP) -c $< -o $@

$(TEST_TARGET): %: %.o $(LIB)
	$(CC) $(CFLAGS) $(SIMD) $(OMP) $< -L. -ldeciphon -lh3client -lh3result -lhmmer_reader -limm -llio -llite_pack -lm -o $@

check_match: test_match
	./test_match

check_model: test_model
	./test_model

check_press: test_press
	./test_press

check_protein: test_protein
	./test_protein

check_scan: test_scan
	./start_daemon.sh minifam.hmm 51300 && ./test_scan   ; e=$$?; ./stop_daemon.sh minifam.hmm; exit $$e

check_massive: test_massive
	./start_daemon.sh massive.hmm 51300 && ./test_massive; e=$$?; ./stop_daemon.sh massive.hmm; exit $$e

check_window: test_window
	./start_daemon.sh minifam.hmm 51300 && ./test_window ; e=$$?; ./stop_daemon.sh minifam.hmm; exit $$e

check: check_match check_model check_press check_protein check_scan check_massive check_window

install: $(LIB) $(HDR)
	@mkdir -p $(PREFIX)/lib $(PREFIX)/include
	install -m 0755 $(LIB) $(PREFIX)/lib/
	install -m 0644 $(HDR) $(PREFIX)/include/

uninstall:
	rm -f $(PREFIX)/lib/$(LIB) $(HDR:%=$(PREFIX)/include/%)

.PHONY: all clean check check_match check_model check_press check_protein check_scan check_massive check_window uninstall
clean:
	rm -f $(OBJ) $(LIB) $(TEST_OBJ) $(TEST_TARGET) *.d
