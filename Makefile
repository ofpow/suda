run:
	@cc -o suda suda.c && ./suda foo.suda
d:
	@cc suda.c -o suda -g && gdb -tui suda
wall:
	@cc -o suda suda.c -Wall -Wextra -pedantic && ./suda foo.suda
val:
	@cc -o suda suda.c -Wall -Wextra -pedantic && valgrind --leak-check=full --show-leak-kinds=all ./suda foo.suda
