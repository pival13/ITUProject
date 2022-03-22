#! /usr/bin/env python3

import json

_A = ord('A')
_charProba = json.load(open('letters.json'))
_bigramsProba = json.load(open('bigrams.json'))

def charOpe(c1, c2):
    return chr(_A + (ord(c1.upper()) - ord(c2) + 26) % 26)

def _cryptanalysis_autokey_perm(cipher: str, keySize: int):
    key = ""
    best = 0
    for n in range(keySize):
        key += ' '
        best = 0
        for i in range(26):
            v = 0
            k = chr(_A+i)
            for pos in range(min((len(cipher)-n)//keySize, 100)):
                k = charOpe(cipher[pos*keySize+n], k)
                v += _charProba[k]
            if v > best:
                best = v
                key = key[:-1] + chr(_A+i)

    print(key)
    order = ['']*keySize
    scores = [0]*keySize
    for first in range(keySize):
        for second in range(keySize):
            if first == second: continue
            v = 0
            autokey = key[first] + key[second]
            for pos in range(min((len(cipher)-n)//keySize, 100)):
                c1 = charOpe(cipher[pos*keySize+first], autokey[0])
                c2 = charOpe(cipher[pos*keySize+second], autokey[1])
                autokey = c1+c2
                if autokey in _bigramsProba:
                    v += _bigramsProba[autokey]
            if v > scores[first]:
                scores[first] = v
                order[first] = second
    
    _key = [(i,key[i],key[n]) for i,n in enumerate(order)]
    lastI = sorted(_key, key=lambda v: scores[v[0]])[0][0]
    key = _key[lastI][1]
    order = [lastI]*keySize
    _key[lastI] = (0,'','')
    for i in range(keySize-1):
        for j,first,second in _key:
            if second == key[0]:
                key = first + key
                order[keySize-2-i] = j
                _key[j] = (0,'','')
                break

    return key, order

def cryptanalysis_autokey_perm(cipher: str):
    plain = ""
    key = ""
    order = []
    best = 1
    for keySize in range(2, 64):
        decrypt = ""
        v = 0
        supposedKey, supposedOrder = _cryptanalysis_autokey_perm(cipher, keySize)
        for n in range(len(cipher)//keySize*keySize):
            decrypt += charOpe(cipher[n // keySize * keySize + supposedOrder[n % keySize]], supposedKey[n] if n < len(supposedKey) else decrypt[n-len(supposedKey)])
            if decrypt[-2:] in _bigramsProba:
                v += _bigramsProba[decrypt[-2:]]
        if v > best:
            best = v
            key = supposedKey
            order = supposedOrder
            plain = decrypt

    return plain, key, order

if __name__ == '__main__':
    from sys import argv
    if len(argv) == 2:
        print(cryptanalysis_autokey_perm(open(argv[1], 'r').read()))
    elif len(argv) == 3:
        cipher = open(argv[1], 'r').read()
        keySize = int(argv[2])
        key, order = _cryptanalysis_autokey_perm(cipher, keySize)
        plain = ""
        for n in range(len(cipher) // keySize * keySize):
            plain += charOpe(cipher[n // keySize * keySize + order[n % keySize]], key[n] if n < len(key) else plain[n-len(key)])
        print((plain, key, order))