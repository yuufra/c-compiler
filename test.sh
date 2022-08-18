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

assert 3 "1 + 2"
assert 1 "1+   4- 3 -  1"
assert 14 "1*2+3*4"
assert 1 "3-4/2"
assert 13 "(1+2)*3+4"
assert 12 "+3*4"
assert 2 "-1*-2"

echo passed!!