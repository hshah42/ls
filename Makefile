CC=cc
CFLAGS= -g -Wall -Werror -Wextra -Wformat=2 -Wjump-misses-init -Wlogical-op -Wpedantic -Wshadow -o 
OUTPUT_PROG=ls
MATH_FLAG= -lm

all: ls

ls: ls.o print.o compare.o
	${CC} ${CFLAGS} ${OUTPUT_PROG} ls.o print.o compare.o ${MATH_FLAG}

ls.o: ls.h common.h print.h compare.h
	${CC} -c ls.c

print.o: common.h
	${CC} -c print.c

compare.o: common.h
	${CC} -c compare.c

debug:
	cc -g -o ls ls.c print.c compare.c -lm
clean:
	rm ls