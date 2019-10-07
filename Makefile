ls:
	cc -Wall -Werror -Wextra -o ls ls.c print.c compare.c
debug:
	cc -g -o ls ls.c print.c compare.c
clean:
	rm ls