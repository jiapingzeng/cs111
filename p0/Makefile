# NAME: Jiaping Zeng
# EMAIL: jiapingzeng@ucla.edu
# ID: 905363270

CC = gcc
CFLAGS = -g -Wall -Wextra

dist_files = lab0.c Makefile README backtrace.png breakpoint.png

default: lab0

check:
	make clean && make
	printf '\nCompiled successfully. Beginning test 1...\n\n'
	echo 'test1test1test1' | ./lab0 | grep '^test1test1test1$$' && printf '\nTest 1 completed successfully. Beginning test 2...\n\n'
	echo 'test2test2test2' | ./lab0 --output test.out | cat test.out | grep '^test2test2test2$$' && rm -f test.* && printf '\nTest 2 completed successfully. Beginning test 3...\n\n'
	echo 'test3test3test3' > test.in | ./lab0 --input test.in | grep '^test3test3test3$$' && rm -f test.* && printf '\nTest 3 completed successfully. Beginning test 4...\n\n'
	echo 'test4test4test4' > test.in | ./lab0 --input test.in --output test.out | cat test.out | grep '^test4test4test4$$' && rm -f test.* && printf '\nTest 4 completed successfully. Beginning test 5...\n\n'
	./lab0 --segfault || echo $$? | grep '^139$$' && printf '\nTest 5 completed successfully. Beginning test 6...\n\n'
	./lab0 --segfault --catch 2>test.err || echo $$? | grep '^4$$' && cat test.err | grep '^Caught segmentation fault$$' && rm -f test.* && printf '\nTest 6 completed successfully. Running make clean...\n\n'
	make clean
	printf '\nAll tests completed successfully.\n'

clean:
	rm -rf *test* *.out *.tar.gz lab0

dist: lab0-905363270.tar.gz

lab0:
	$(CC) $(CFLAGS) lab0.c -o $@

lab0-905363270.tar.gz:
	tar -czf $@ $(dist_files)