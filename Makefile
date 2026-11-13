CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -Iinclude
MODULES = src/priority_queue.c src/scheduler.c src/analysis.c src/report.c src/file_handler.c src/validation.c
SRC = $(MODULES) src/main.c
OBJ = $(SRC:.c=.o)

ifeq ($(OS),Windows_NT)
	TARGET = scheduler_sim.exe
	TEST_TARGET = test_runner.exe
	RM = del /Q /F
	FIX_PATH = $(subst /,\,$1)
	SIGN_TARGET = powershell -NoProfile -Command "$$c = Get-ChildItem Cert:\CurrentUser\Root | Where-Object {$$_.Subject -eq 'CN=LocalDev'}; if ($$c) { Set-AuthenticodeSignature -FilePath '.\$(TARGET)' -Certificate $$c }"
	SIGN_TEST = powershell -NoProfile -Command "$$c = Get-ChildItem Cert:\CurrentUser\Root | Where-Object {$$_.Subject -eq 'CN=LocalDev'}; if ($$c) { Set-AuthenticodeSignature -FilePath '.\$(TEST_TARGET)' -Certificate $$c }"
	RUN_CMD = .\$(TARGET)
	RUN_TEST = .\$(TEST_TARGET)
else
	TARGET = scheduler_sim
	TEST_TARGET = test_runner
	RM = rm -f
	FIX_PATH = $1
	SIGN_TARGET = @true
	SIGN_TEST = @true
	RUN_CMD = ./$(TARGET)
	RUN_TEST = ./$(TEST_TARGET)
endif

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) -lm
	$(SIGN_TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	$(RUN_CMD)

test: $(MODULES) tests/test_runner.c
	$(CC) $(CFLAGS) $(MODULES) tests/test_runner.c -o $(TEST_TARGET) -lm
	$(SIGN_TEST)
	@echo "Test runner compiled and signed successfully. Run '.\test_runner.exe' manually."

clean:
	-$(RM) $(call FIX_PATH,src/*.o) $(TARGET) $(TEST_TARGET) *.txt 2>nul || true

.PHONY: all run test clean