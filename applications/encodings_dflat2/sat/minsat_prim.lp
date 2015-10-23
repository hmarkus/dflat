length(2). or(0). and(1).

% Guess root-to-leaf paths in item trees to be extended
extend(0,R) :- root(R).
1 { extend(L+1,S) :  sub(R,S) } 1 :- extend(L,R), L<2.

% Guess truth value of introduced atoms
{ item(2,A;1,A) : introduced(A) }.

% Only join root-to-leaf paths coinciding on atom truth values
f(S,X)     :- atNode(S,N), sub(_,S), childNode(N), bag(N,X), not childItem(S,X).
:- extend(L,X;L,Y), childItem(X,A), f(Y,A), L=1..2.

% True atoms are kept
item(L,X)    :- extend(L,R), childItem(R,X),    current(X), L=1..2.

% Interpretation at level 2 must be subset of that at level 1
:- item(2,A), not item(1,A).

% Update subset information; reject larger models at root
auxItem(2,smaller) :- extend(2,S), childAuxItem(S,smaller).
auxItem(2,smaller) :- item(1,A), not item(2,A).
reject :- final, auxItem(2,smaller).
accept :- final, not reject.

% Current clauses
notCurr(C) :- occursIn(A,C), not current(A).
curr(C) :- clause(C), not notCurr(C).

% Define satisfied clauses
sat(L,C) :- curr(C), pos(C,A), item(L,A), L=1..2.
sat(L,C) :- curr(C), neg(C,A), not item(L,A), L=1..2.

% Current clauses need to be satisfied
:- curr(C), not sat(L,C), L=1..2.

#show auxItem/2.
#show item/2.
#show extend/2.
#show length/1.
#show or/1.
#show and/1.
#show accept/0.
#show reject/0.

