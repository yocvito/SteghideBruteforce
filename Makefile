CC=gcc
FLAGS=-Wall -Wextra -Werror -g
SLIBS=-lpthread
src=${wildcard *.c}

all: steghide-bruteforce 
#get-error-output

steghide-bruteforce: $(src)
	$(CC) $^ -o $@ $(FLAGS) $(SLIBS)

#get-error-output: get-stegerr.c
#	$(CC) $^ -o $@ $(FLAGS)

.PHONY: clean
clean:
	rm -rf steghide-bruteforce 
# 	get-error-output
