testing:
	gcc -g3 -Wall -Wextra -Iinc -o testing.out ./test/test.c ./src/*.c -pthread