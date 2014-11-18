length(2).
valid(1..2).
or(L-1) :- valid(L).
accept.

% Make explicit that a row interprets an atom as false or a clause as unsatisfied
false(R,X) :- atNode(R,N), childNode(N), bag(N,X), not childItem(R,X).
false(S,X) :- atNode(R,N), childNode(N), bag(N,X), sub(R,S), not childItem(S,X).
unsat(R,C) :- atNode(R,N), childNode(N), bag(N,C), not childAuxItem(R,C).
unsat(S,C) :- atNode(R,N), childNode(N), bag(N,C), sub(R, S), not childAuxItem(S,C).

1 { extend(0,R) : rootOf(R,N) } 1 :- childNode(N).
1 { extend(L+1,S) : sub(R,S) } 1 :- extend(L,R), L<2.

% Only join rows that coincide on common atoms
:- extend(L,X;L,Y), atom(A), childItem(X,A), false(Y,A), valid(L).

% Some child rows cannot be extended since they are killed
:- extend(L, R), unsat(R), valid(L).
unsat(R) :- clause(C), removed(C), unsat(R,C).

% True atoms and satisfied clauses remain so unless removed
item(L, X) :- extend(L, R), childItem(R,X), current(X), valid(L).
auxItem(L, C) :- extend(L, R), childAuxItem(R,C), current(C), valid(L).

% Level 2 interpretation must not be bigger than level 1
:- atom(A), item(2,A), not item(1,A).

% Guess truth value of introduced atoms
{ item(2,A;1,A) : atom(A) , introduced(A) }.
:- final, extend(1, R), sub(R, S), childAuxItem(S, smaller), not unsat(S).

auxItem(2,smaller) :- extend(2,S), childAuxItem(S,smaller).
auxItem(2,smaller) :- atom(A), item(1,A), not item(2,A).

% Through the guess, clauses may become satisfied
auxItem(L,C) :- current(C;A), pos(C,A), item(L,A),valid(L).
auxItem(L,C) :- current(C;A), neg(C,A), not item(L,A), valid(L).

#show item/2.
#show auxItem/2.
#show length/1.
#show extend/2.
#show accept/0.
#show reject/0.
#show or/1.
