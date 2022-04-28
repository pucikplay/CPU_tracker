testing:
	gcc -Wall -Wextra -Iinc -o testing.out ./test/test.c ./src/stat_reader.c -pthread