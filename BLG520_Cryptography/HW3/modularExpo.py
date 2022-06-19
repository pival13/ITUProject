#! /usr/bin/env python3

def modPow(b, e, m):
    if m == 1:
        return 0
    r = 1
    b %= m
    while e > 0:
        if e % 2 == 1:
            r = (r*b) % m
        b = (b*b) % m
        e >>= 1
    return r