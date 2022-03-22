#! /usr/bin/env python3

import json

_A = ord('A')
_bigramsProba = json.load(open('bigrams.json'))

def charOpe(c1, c2):
    return chr(_A + (ord(c1.upper()) - ord(c2) + 26) % 26)

def _cryptanalysis_autokey(cipher: str, keySize: int):
    key = ""
    best = 0
    for i in range(26):
        for j in range(26):
            v = 0
            autokey = chr(_A+i)+chr(_A+j)
            for pos in range(min(int((len(cipher)-1)/keySize), 100)):
                autokey = charOpe(cipher[pos*keySize], autokey[0]) + charOpe(cipher[pos*keySize+1], autokey[1])
                if autokey in _bigramsProba:
                    v += _bigramsProba[autokey]
            if v > best:
                best = v
                key = chr(_A+i)+chr(_A+j)

    for n in range(2,keySize):
        key += ' '
        best = 0
        for i in range(26):
            v = 0
            autokey = key[-2:-1]+chr(_A+i)
            for pos in range(min(int((len(cipher)-n)/keySize), 100)):
                first = charOpe(cipher[pos*keySize+n-1], autokey[0])
                second = charOpe(cipher[pos*keySize+n], autokey[1])
                autokey = first+second
                if autokey in _bigramsProba:
                    v += _bigramsProba[autokey]
            if v > best:
                best = v
                key = key[:-1] + chr(_A+i)

    return key

def cryptanalysis_autokey(cipher: str):
    plain = ""
    key = ""
    best = 0
    for keySize in range(2, 64):
        decrypt = ""
        v = 0
        supposedKey = _cryptanalysis_autokey(cipher, keySize)
        print(supposedKey)
        for n in range(len(cipher)):
            decrypt += charOpe(cipher[n], supposedKey[n] if n < len(supposedKey) else decrypt[n-len(supposedKey)])
            if decrypt[-2:] in _bigramsProba:
                v += _bigramsProba[decrypt[-2:]]
        if v > best:
            best = v
            key = supposedKey
            plain = decrypt

    return plain, key

if __name__ == '__main__':
    from sys import argv
    if len(argv) == 2:
        print(cryptanalysis_autokey(open(argv[1], 'r').read()))
    elif len(argv) == 3:
        cipher = open(argv[1], 'r').read()
        key = _cryptanalysis_autokey(cipher, int(argv[2]))
        plain = ""
        for n in range(len(cipher)):
            plain += charOpe(cipher[n], key[n] if n < len(key) else plain[n-len(key)])
        print((plain, key))