%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% MinSAT                                   %
% using non-normalized tree decompositions %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

length(2).
level(1..2).
or(0).
and(1).

% Make explicit that a row interprets an atom as false or a clause as unsatisfied
false(S,X) :- atNode(S,N), childNode(N), bag(N,X), sub(_,S), not childItem(S,X).
unsat(S,C) :- atNode(S,N), childNode(N), bag(N,C), sub(_,S), not childAuxItem(S,C).

extend(0,R) :- root(R).
1 { extend(L+1,S) : sub(R,S) } 1 :- extend(L,R), L<2.

% Only join rows that coincide on common atoms
:- extend(L,X;L,Y), atom(A), childItem(X,A), false(Y,A), level(L).

% Some child rows cannot be extended since they are killed
:- extend(L, R), unsat(R), level(L).
unsat(R) :- clause(C), removed(C), unsat(R,C).

% True atoms and satisfied clauses remain so unless removed
item(L, X) :- extend(L, R), childItem(R,X), current(X), level(L).
auxItem(L, C) :- extend(L, R), childAuxItem(R,C), current(C), level(L).

% Level 2 interpretation must not be bigger than level 1
:- atom(A), item(2,A), not item(1,A).

% Guess truth value of introduced atoms
{ item(2,A;1,A) : atom(A) , introduced(A) }.
reject :- final, extend(1, R), sub(R, S), childAuxItem(S, smaller), not unsat(S).
accept :- final, not reject.

auxItem(2,smaller) :- extend(2,S), childAuxItem(S,smaller).
auxItem(2,smaller) :- atom(A), item(1,A), not item(2,A).

% Through the guess, clauses may become satisfied
auxItem(L,C) :- current(C;A), pos(C,A), item(L,A), level(L).
auxItem(L,C) :- current(C;A), neg(C,A), not item(L,A), level(L).

#hide.
#show item/2.
#show auxItem/2.
#show length/1.
#show extend/2.
#show accept/0.
#show reject/0.
#show or/1.
#show and/1.
