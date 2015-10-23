% Guess partial solutions to be extended
1 { extend(R) : childRow(R,N) } 1 :- childNode(N).

% Guess truth value of introduced atoms
{ item(A) : introduced(A) }.

% Only join rows coinciding on truth values of atoms
f(R,X)     :- childRow(R,N), bag(N,X), not childItem(R,X).
:- extend(X;Y), childItem(X,A), f(Y,A).

% True atoms are kept
item(X)    :- extend(R), childItem(R,X),    current(X).

% Current clauses
notCurr(C) :- occursIn(A,C), not current(A).
curr(C) :- clause(C), not notCurr(C).

% Define satisfied clauses
sat(C) :- curr(C), pos(C,A), item(A).
sat(C) :- curr(C), neg(C,A), not item(A).

% Current clauses need to be satisfied
:- curr(C), not sat(C).

% Optimize with respect to the item set
optItem(X) :- item(X).

#show item/1.
#show extend/1.
#show optItem/1.

