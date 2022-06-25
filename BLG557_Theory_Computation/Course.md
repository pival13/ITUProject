relation = subset of a cartesian product between two set

function = relation where every input have a single output

non-deterministic machien = a same state and input can lead to different output

Alphabet = $\Sigma$
Empty string = $\Lambda$ or $\epsilon$
Set of non-empty string = $\Sigma^+$
$\Sigma^+ + \Lambda => \Sigma^*$

commutativity = a b == b a
associativity = (a b) c == a (b c)

Language $A$ => $A^+$ = $\cup_{n=1}A$, $A^*$ = $\{\Lambda\} \cup A^+$

$A,B \in \Sigma^* \wedge \Lambda \not \in A$ => $X = AX \cup B \Rightarrow X = A^*B$

$X = XA \cup B \iff X = BA^*$

## Deterministic Finite State Machine

No path with twice the same condition

## Non-Deterministic Finite State Machine

An execution fail iff every possible execution path fail

NFA can be converted to DFA by combining states with same path or lambda path

## Glossary

Grammar:    State => (State . Character) *
Expression of language:   character + numerators (*+|)

DFA: Deterministic Formal Automata
NFA: Non-Deterministic Formal Automata
PDA: Push-Down Automata
LBA: Linear-Bounded Automaton
TM: Turing Machine
ATM: Accesptance Turing Machine

Regular Language: State => (character . State) | Lambda
CFG: Context Free Language: State => (Character | State) *
CNF: Chomsky normal form: State => Character | (State . State)

Turing recognizable <=> recursively enumarble <=> can return a yes answer
Turing decidable    <=> recursive             <=> can return a yes/no answer

## Chomsky hierarchy

Turing-recognizable > Turing-decidable > Context sensitive > Context free > Deterministic context free > Regular
TM                  > TM               > LBA               > PDA          > PDA                        > DFA

N-TM is Polynomial if it is verifiable in P-time

NP-complete: SAT reducible
NP-Intermediate: Likely P, but no proof of such



1-9 + 12 + 14