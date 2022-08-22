#!/bin/bash
assert(){
    # ""はエスケープ($,`,\を除く)
    expected="$1"
    input="$2"

    ./compiler "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual="$?"

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
assert 15 "foo = 3; bar = 4; foo + bar * foo;"

echo passed!!