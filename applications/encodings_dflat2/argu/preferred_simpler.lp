%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% preferred semantics simplified           %
% using non-normalized tree decompositions %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

length(2). or(0). and(1).

% Guess root-to-leaf paths in item trees to be extended
extend(0,R) :- root(R).
1 { extend(L+1,S) :  sub(R,S) } 1 :- extend(L,R), L<2.

% Guess truth value of introduced atoms
0 { item(L,in(A)) } 1 :- introduced(A), L=1..2.

% Items at level 2 must be subset of that at level 1
item(2,A) :- item(1,A).

% Only join root-to-leaf paths coinciding on colors
nIn(S,A)  :- childNode(N), atNode(S,N), bag(N,A), not childItem(S,in(A)).
:- extend(L,S1;L,S2), childItem(S1,in(A)),      nIn(S2,A).

% Inherit arguments that are in, defeated or attackers
item(L,in(A))     :- extend(L,S), childItem(S,in(A)),     current(A).
auxItem(L,def(A)) :- extend(L,S), childAuxItem(S,def(A)), current(A).
% Set defeated arguments
auxItem(L,def(A)) :- current(A;B), att(B,A), item(L,in(B)).

% Still remaining (undefeated) attackers
auxItem(L,att(A)) :- extend(L,S), childAuxItem(S,att(A)), current(A), not auxItem(L,def(A)).
auxItem(L,att(A)) :- current(A;B), att(A,B), item(L,in(B)), not auxItem(L,def(A)).

% Assure that the set is conflict-free  
:- item(L,in(A)), item(L,in(B)), att(A,B).

% Remove candidates that leave attackers undefeated
:- extend(L,S), childAuxItem(S,att(A)), removed(A).

% Update subset information; reject models at root with larger s
auxItem(2,bigger) :- extend(2,S), childAuxItem(S,bigger).
auxItem(2,bigger) :- item(2,A), not item(1,A).	%bigger S
reject :- final, auxItem(2,bigger).
accept :- final, not reject.

#show length/1.
#show or/1.
#show and/1.
#show reject/0.
#show accept/0.
#show item/2.
#show auxItem/2.
#show extend/2.
