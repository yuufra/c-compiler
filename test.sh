#!/bin/bash
assert(){
    # ""はエスケープ($,`,\を除く)
    expected="$1"
    input="$2"

    ./compiler "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual="$?" # Unixのプロセス終了コードは0〜255なのでactualの取る値も同じ

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 10 "10;"
assert 13 "(1+2)*3+4;"
assert 0 "(1==3)+4*2<5;"
assert 5 "a=1;c=2;a+2*c;"
assert 14 "a = 3; b = 5 * 6 - 8; a + b / 2;"
assert 15 "foo = 3; bar = 4; return foo + bar * foo;"
assert 2 "if (1) 2; else 3;"
assert 2 "if (1) 2;"
assert 2 "while (1) return 2;"
assert 3 "foo = 10; while (foo!=3) foo = foo - 1; return foo;"
assert 10 "for(i=1; i<10; i=i+1) return 10;"
assert 10 "foo=1; for(i=1; i<10; i=i+1) foo=foo+1; return foo;"
assert 2 "{1;2;}"
assert 1 "if(1){10;20;} return 1;"
assert 1 "if(1){} return 1;"
assert 3 "foo=10; cnt=0; while(foo>3){foo=foo-3; cnt=cnt+1;} return cnt;"

echo passed!!