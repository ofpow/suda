run:
	@cc -o suda src/suda.c -Wall -Wextra && ./suda -a foo.suda
d:
	@cc src/suda.c -o suda -g -Wall -Wextra && gdb -tui suda
val:
	@cc -o suda src/suda.c -Wall -Wextra -g && valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./suda -a foo.suda
