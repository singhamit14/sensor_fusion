# Host unit tests (MinGW gcc). Run from test/host:  mingw32-make
# STM32CubeCLT puts sh.exe on PATH; force cmd so .\test_x works.
SHELL = cmd.exe
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -g -I../../Lib/Inc -I.

TESTS = test_ringbuf

all: run

# cmd.exe is the shell under mingw32-make; && chain stops on first failure.
run: $(TESTS)
	@$(foreach t,$(TESTS),cmd /c "echo == $(t) ==" && .\$(t) &&) cmd /c "echo ALL SUITES PASSED"

test_ringbuf: test_ringbuf.c ../../Lib/Src/ringbuf.c
	$(CC) $(CFLAGS) -DHOST_TEST $^ -o $@

clean:
	cmd /c "del /Q *.exe 2>nul & exit 0"