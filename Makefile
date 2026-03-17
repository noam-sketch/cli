CC = gcc
CFLAGS = `pkg-config --cflags gtk+-3.0 vte-2.91 fontconfig` -Wall -Wextra -g -fprofile-arcs -ftest-coverage
LDFLAGS = `pkg-config --libs gtk+-3.0 vte-2.91 fontconfig` -lgcov -rdynamic

all: cli

cli: main.c
	$(CC) $(CFLAGS) -o cli main.c $(LDFLAGS)

test: cli_test
	./cli_test
	gcov main.c

cli_test: main.c tests/test_main.c
	# Compile test_main which includes main.c
	$(CC) $(CFLAGS) -DTEST_MODE -o cli_test tests/test_main.c $(LDFLAGS)

clean:
	rm -f cli cli_test *.gcda *.gcov *.gcno
