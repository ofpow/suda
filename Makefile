run:
	@cc -o suda suda.c -Wall -Wextra && ./suda foo.suda
d:
	@cc suda.c -o suda -g -Wall -Wextra && gdb -tui suda
val:
	@cc -o suda suda.c -Wall -Wextra -g && valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./suda foo.suda
