## Test

$$
\text{a}\ \mathbf{test} \times
\iff \to \gets \mapsto \Rightarrow
\in \forall \exists \nexists
\wedge \vee

\begin{equation}
    a
\end{equation}

\begin{cases}
    a & b \\
    c & d
\end{cases}

\begin{matrix}
    a & b \\
    c & d
\end{matrix}
$$

## Euclidean algorithm

$$ gcd(a,b) = gcd(b, a \mod b) $$

## Arithmetic with large number


$$ gcd(2^a-1,2^b-1) = 2^{\gcd(a,b)}-1 $$
$$ (2^a-1) \mod (2^b-1) \equiv 2^{a \mod b}-1 $$

## Fibonacci numbers

$$ f_k = \begin{cases} 1 & k=0 \\ 1 & k=1 \\ f_{n-1} + f_{n-2} & k=n \end{cases} > \left(\frac{1+\sqrt 5}{2}\right)^{n-1} $$
$\frac{1+\sqrt 5}{2}$ is the golden ratio $\alpha$.

## Ferat's theorem

$$ p\text{ is prime, } \nexists i \in [1,(p-2)],\ i \equiv -1\text{ mod }p $$

$$ a^{p-1} \equiv 1\text{ mod }p \wedge p\text{ is prime}\wedge a \bot p $$

## Pseudoprimes

$$ 2^{n-1} \equiv 1 \text{ mod } n \Rightarrow n\text{ is prime} $$

<!--
2^117 ?= 2 mod 117
    2^117 = (2^7)^16*2^5
    2^7 mod 117 = 11
    4 mod 117 = 121
11^16*2^5 mod 117 = 4^8*2^5 mod 117 = (2^7)^3 mod 117 = 11^3 mod 117 = 44 mod 117
117 not pseudoprime

2^340 ?= 1 mod 341
    2^340 = 1 mod 11
    2^340 = 1 mod 31
    => 341 pseudoprime but not prime

561 is pseudoprime ? (561 = 3*11*17)
    a^560 ?= 1 mod 3 => (a^2)^280 mod 3 = 1
    a^560 ?= 1 mod 11 =>(a^10)^36 mod 3 = 1
    a^560 ?= 1 mod 17 => (a^16)^35 mod 3 = 1
-->

## Linear congruence

$$ ax \equiv b \text{ mod } p \wedge \gcd(a,p) = 1 $$
$$ a^{p-1} \equiv 1 \text{ mod } p $$
$$ aa^{p-1} \equiv 1 \text{ mod } p $$
$$ \bar{a} = a^{p-2} $$
$$ x \equiv a^{p-2}b \text{ mod } p$$

<!--
29^1000 mod 37 ?= x
    1000 = 36*27+28
(29^36)^27 * 29^28 mod 37
    28
-->

## Euler's totient function

$$ \forall m \in \mathbf{N}, \phi(m) \text{ is size of set with x < m and gcd(x,m) = 1} $$
<!-- m=10, set={1,3,7,9}, phih(10)=4 -->
$$ \phi(m) = m-1 \iff m \text{ is prime} $$
$$ \phi(m^a) = m^a-m^{a-1} = m^a(1-1/m) \iff m \text{ is prime} \wedge a \in \mathbf{N} $$
$$ \phi(ab) = \phi(a)\times\phi(b) \wedge a \perp b $$

$$ \forall n \in \mathbf{N}, \phi(n) = n\prod_{p_i|n}{1-1/pi} $$
<!--
phi(45)     45 = 3^2*5
    45 * (1-1/3) * (1-1/5)
    3*3*5 * 2/3 * 4/5
    3*2*4
    24
-->

## Euler-Fernant theorme

$$ \huge{\text{IMPORTANT}} $$

$$ \gcd(a,m) = 1 \Rightarrow a^{\phi(m)} \equiv 1 \text{ mod } m$$ 

$$ x \perp a \wedge x \perp b \iff x \perp ab $$

### Exemple of used for RSA (encryption):

p,q 2 large prime number.n = pq => $\phi(n) = (p-1)(q-1)$

Let's take e,d, s.t. e.d = 1 mod $\phi(n)$. e = public key, d = private key

Encryption:
m = message. c = $m^e$ mod n. m < p < q => m $\perp$ n

Decryption:
$c^d \equiv m^{ed}\text{ mod }n \Rightarrow c^d \equiv m^{1+k\phi(n)}\text{ mod }n \Rightarrow c^d \equiv m\text{ mod }n$
<!-- //TODO test this -->

<!-- 
a=5, b=9
x = u mod 9
x = v mod 5
x = uM_9y_9 + vM_5y_5
  = u*5*2 + v*9*4
  = 10u + 36v
 -->

## Counting

$A$ a finite set $\iff \forall a \in A, \exist!i \in \{1...n\} \wedge f(a) = i \wedge f^{-1}(a) = i$ 

Nb element of $A$ = $|A|$= $Card(a)$

$$ A \cap B = \empty $$
$$ |A \cup B| = m+n, |A|=m, |B|=n $$

$$ |A_1 \cup A_2 \cup ... A_n| = (-1)^0\sum|A_i|+(-1)^1\sum|A_i\cap A_j + ... + (-1)^{n-1}\sum|A_1 \cap A_2 \cap ... A_n| $$

$$ \text{number of function } A \rightarrow B = |B|^{|A|} = n^m $$

$$ B \subseteq A, \forall a \in B \Rightarrow a \in A $$

$$ P(a) \text{ (Powerset of A)}. |A|=n, |P(A)|=2^n $$

Combination without repetition:
$$ |A|=n, |B|=r, B\subseteq A => C(n,r) = \frac{n!}{r!(n-r)!} $$

With repetition:
$$ = C(r+n-1,n-1) = C(r+n-1,r) = \frac{(r+n-1)!}{r!(n-1)!} $$

### Permutation

$$ A = B \cup C, B \cap C = \empty $$
$$ B = \{\} $$

## Pascal's Theorem

$$ C(n+1,k) = C(n,k-1) + C(n,k) $$

$$ \sum_i^n C(n,i) = 2^n $$

## Vandermand Convolution

$$ C(m+n, r) = \sum{C(m,r-i).C(n,i) } $$

## Binomial Theorem

$$ (x+y)^n = \sum{C(n,i)x^{n-i}y^{i}} $$

## Multinomial Theorem

$$ (x_1+x_2+x_3+...x_m)^n = \sum C(n, n_1*n_2*n_3*...n_m)x^{n_1}x^{n_2}...x^{n_m} $$

## Stirling numbers of the second kind

How many combination the set $A$ can be divided into $n$ non-empty set.



$$ S(n,k) = kS(n-1,k) + S(n-1,k-1) = N(m,n)/n! $$



## Burnside theorem

Number of equivalence classes (on permutation) = Sum of number of unchanged elements divided by number of permutations




$\newcommand{\congru}[3]{#1 \equiv #2\ (\text{mod}\ #3)}$

$x = \sum_{i=1}^{n}{a_iM_iy_i} \wedge \congru{M_i y_i}{1}{m} \wedge m = \prod{m_i} \wedge M_i = m/m_i$

$$ \congru{a_nx}{b_n}{m_n} ... $$

\if false
15x = 21 mod 48
166x = 46 mod 22
x = 5 mod 13

5x = 7 mod 16
    16 = 5*3 + 1
        1 = 16 - 3*5
-3*5x = -3*7 mod 16
x = 11 mod 16

83x = 23 mod 11
    83 = 7*11 + 6
    11 = 1*6+5
    6 = 1*5+1
        1 = 2*83-15*11
2*83x = 2*23 mod 11
x = 2 mod 11

m = 16*11*13 = 2288
M_16 = 143, M_11 = 208, M_13 = 176
\fi
