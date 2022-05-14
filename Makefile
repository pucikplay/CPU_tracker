RM := rm -rf

SRC := $(wildcard ./src/*.c)

CC ?= gcc

C_FLAGS := -Wall -Wextra

ifeq ($(CC),clang)
	C_FLAGS += -Weverything -Wno-disabled-macro-expansion
endif

all: main.out

main.out:
	$(CC) $(C_FLAGS) -Iinc ./app/main.c $(SRC) -o $@ -pthread

clean:
	$(RM) main.out