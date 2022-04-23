#! /usr/bin/env python3

from bitarray import bitarray
import math

SUBST = [0x0,0x1,0x9,0x6,0xD,0x7,0x3,0x5,0xF,0x2,0xC,0xE,0xA,0x4,0xB,0x8,0x0]
PERM = [0,4,8,12,1,5,9,13,2,6,10,14,3,7,11,15]


# Encryption

def str2block(s):
    return [bitarray(('0'*16+bin(s)[2:])[-16:])]

def block2str(blocks):
    s = b''
    for bits in blocks:
        s += bits.tobytes()
    return blocks

def schedule(masterKey):
    key = bitarray()
    key.frombytes(masterKey)
    while True:
        yield key
        k0 = key[15] ^ key[13] ^ key[11] ^ key[10]
        key = bitarray(bin(k0)[2:]) + key[:-1]

fromBin = lambda bits,i: int(bits[4*i:4*i+4].to01(),2)
toBin = lambda n: bitarray(('0'*4+bin(n)[2:])[-4:])

def SPN(plain, sbox, pbox, rounds):
    secretKey = b'\xe4\x9e'
    blocks = str2block(plain)
    keys = schedule(secretKey)

    for i in range(0,rounds):
        key = next(keys)
        if i < rounds-1:
            blocks = [pbox(sbox(block ^ key)) for block in blocks]
        else:
            blocks = [sbox(block ^ key) for block in blocks]
    key = next(keys)
    blocks = [block ^ key for block in blocks]

    return block2str(blocks)

def cipher1(plain):
    sbox = lambda bits: toBin(SUBST[fromBin(bits,0)+1]) + toBin(SUBST[fromBin(bits,1)+1]) + toBin(SUBST[fromBin(bits,2)+1]) + toBin(SUBST[fromBin(bits,3)+1])
    pbox = lambda bits: bitarray([bits[i] for i in PERM])
    return SPN(plain, sbox, pbox, 4)

def cipher2(plain):
    sbox = lambda bits: toBin(SUBST[fromBin(bits,0)]) + toBin(SUBST[fromBin(bits,1)]) + toBin(SUBST[fromBin(bits,2)]) + toBin(SUBST[fromBin(bits,3)])
    pbox = lambda bits: bitarray([bits[i] for i in PERM])
    return SPN(plain, sbox, pbox, 4)

def cipher3(plain):
    sbox = lambda bits: toBin(SUBST[fromBin(bits,0)]) + toBin(SUBST[fromBin(bits,1)]) + toBin(SUBST[fromBin(bits,2)]) + toBin(SUBST[fromBin(bits,3)])
    pbox = lambda bits: bits
    return SPN(plain, sbox, pbox, 4)


# Differential cryptanalysis

def differentialTable(substTable):
    dy = {}
    for dx in range(0b0000,0b1111+1):
        dy[dx] = {}
        for x in range(0b0000,0b1111+1):
            dy[dx][x] = substTable[x] ^ substTable[x^dx]

    diffTbl = {}
    for x in range(0b0000,0b1111+1):
        diffTbl[x] = {}
        for y in range(0b0000,0b1111+1):
            l = len([True for d in dy[x].values() if d==y])
            if l > 0: diffTbl[x][y] = l

    return diffTbl

def findBestDelta(substTbl, desiredLastBox=None):
    toBit = lambda n: bitarray(('0'*16 + bin(n)[2:])[-16:])
    diffTbl = differentialTable(substTbl)

    maxP = 0
    maxDx = toBit(0)
    maxDy = 0
    maxBox = 0

    # The permutation here is particular:
    #   The input of round N is the output of round N-2
    #   The boxes concerned of round N is the output of round N-1
    # We can resume it as follow:
    #   The N-th Sbox M-th output bit goes to the M-th Sbox N-th input bit.

    # Trying every possible input for 1 box
    for u1 in range(0b0001,0b1111+1):
        # Testing with every possible outputs
        for v1,count1 in diffTbl[u1].items():
            # Testing with every possible box
            for u2 in [0b0001,0b0010,0b0100,0b1000]:
                box1 = 1
                for v2,count2 in diffTbl[u2].items():
                    box2 = bin(v1).count('1')
                    u3 = v1
                    for v3,count3 in diffTbl[u3].items():
                        box3 = bin(v2).count('1')
                        u4 = v2
                        proba = (count1/16)**box1 * (count2/16)**box2 * (count3/16)**box3
                        if proba > maxP and (v3 == desiredLastBox if desiredLastBox else True):
                            maxP = proba
                            maxDx = toBit(u1 << (4*int(math.log2(u2))))
                            maxDy = u4
                            maxBox = v3
    
    return maxDx, maxDy, [i for i in range(4) if maxBox & (1<<i)], maxP

def findBestDelta_noPerm(substTbl, startBox):
    toBit = lambda n: bitarray(('0'*16 + bin(n)[2:])[-16:])
    diffTbl = differentialTable(substTbl)

    maxP = 0
    maxDx = toBit(0)
    maxDy = 0
    boxCount = len([True for i in range(4) if startBox & (1<<i)])

    for u1 in range(0b0001,0b1111+1):
        for v1,count1 in diffTbl[u1].items():
            u2 = v1
            for v2,count2 in diffTbl[u2].items():
                u3 = v2
                for v3,count3 in diffTbl[u3].items():
                    u4 = v3
                    proba = (count1/16)**boxCount * (count2/16)**boxCount * (count3/16)**boxCount
                    if proba > maxP:
                        maxP = proba
                        maxDx = toBit(sum([u1 << (4*i) for i in range(4) if startBox & (1<<i)]))
                        maxDy = u4
    
    return maxDx, maxDy, [i for i in range(4) if startBox & (1<<i)], maxP

def findSubKey(dx, dy, lastBoxes, invSbox, cipherFn):
    toBit = lambda n: bitarray(('0'*16 + bin(n)[2:])[-16:])
    dx = int(dx.to01(),2)
    dy = toBit(sum([dy << (4*i) for i in range(4) if i in lastBoxes]))
    nullMask = toBit(sum([0b1111 << (4*i) for i in range(4) if i not in lastBoxes]))

    counts = {}
    # Trying ~10,000 different input cases
    for x1 in range(0x0000,0xFFFF+1,5):
        x2 = x1^dx
        y1 = cipherFn(x1)[0]
        y2 = cipherFn(x2)[0]

        # Since we don't have permutation here, we can filter the result depending on the bits studied
        if (y1^y2) & nullMask != toBit(0): continue
        
        k = 0
        while True:
            # Testing every key from 0b0000 to 0b1111, for every 4-bit
            key = toBit(sum([((k >> (4*i)) & 0b1111) << (4*box) for i,box in enumerate(lastBoxes)]))
            delta = invSbox(y1 ^ key) ^ invSbox(y2 ^ key)
            if delta == dy:
                if key.to01() in counts: counts[key.to01()] += 1
                else:                    counts[key.to01()] = 1
            if bin(k).count('1') >= len(lastBoxes)*4: break
            k += 1
    return counts

def reveresKeySchedule(subKey, round):
    key = subKey
    for _ in range(round):
        print(key)
        k15 = key[0] ^ key[14] ^ key[12] ^ key[11]
        key = key[1:] + bin(k15)[2:]
    return key


if __name__ == '__main__':
    SUBST_TBL = SUBST[1:] # or SUBST[:-1]
    spn = cipher1 # or cipher2 or cipher3

    INV_SUBST_TBL = {v:i for i,v in enumerate(SUBST_TBL)}
    invSbox = lambda bits: toBin(INV_SUBST_TBL[fromBin(bits,0)]) + toBin(INV_SUBST_TBL[fromBin(bits,1)]) + toBin(INV_SUBST_TBL[fromBin(bits,2)]) + toBin(INV_SUBST_TBL[fromBin(bits,3)])
    
    key = bitarray('0'*16)
    for boxes in [0b1100,0b0110,0b0011,0b1001]:# or [0b1000,0b0100,0b0010,0b0001]
        dx, dy, lastBoxes, p = findBestDelta(SUBST_TBL, boxes) # or findBestDelta_noPerm
        counts = findSubKey(dx, dy, lastBoxes, invSbox, spn)
        subkey = sorted(counts.keys(), key=lambda k:counts[k], reverse=True)[0]
        key = key | bitarray(subkey)
        print(dx, dy, p, lastBoxes, subkey, key)

    key = reveresKeySchedule(key, 4)
    print(key, key.tobytes())
