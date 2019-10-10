ls:
	cc -Wall -Werror -Wextra -o ls ls.c print.c compare.c -lm
debug:
	cc -g -o ls ls.c print.c compare.c -lm
clean:
	rm ls