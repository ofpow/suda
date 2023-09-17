run:
	@cc -o suda suda.c -Wall -Wextra -pedantic && ./suda foo.suda
d:
	@cc suda.c -o suda -g -Wall -Wextra -pedantic && gdb -tui suda
val:
	@cc -o suda suda.c -Wall -Wextra -pedantic -g && valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./suda foo.suda
