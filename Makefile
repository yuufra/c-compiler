CFLAGS=-std=c11 -g -static

compiler:	compiler.c
test:	compiler
		./test.sh

clean:
	rm -f compiler *.o *~ tmp*

# PHONYは疑似ターゲットと呼ばれ、存在しないファイル名を指定できる
.PHONY:	test clean