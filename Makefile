CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

compiler:	$(OBJS)
		$(CC) -o compiler $(OBJS) $(LDFLAGS)

$(OBJS): compiler.h

test:	compiler
		./test.sh

clean:
	rm -f compiler *.o *~ tmp*

# PHONYは疑似ターゲットと呼ばれ、存在しないファイル名を指定できる
.PHONY:	test clean