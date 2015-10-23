%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ASP                                       %
% using semi-normalized tree decompositions %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Make explicit when an atom is false or a rule is unsatisfied
false(R,X) :- childRow(R, N), bag(N,X), not childItem(R,X).
falsea(R,X) :- childRow(R, N), bag(N,X), atom(X), not childItem(R,a(X)).

% Guess a branch in the item tree for every child node
1 { extend(R) : childRow(R,N) } 1 :- childNode(N).

% Only join child item sets that coincide on common atoms
:- extend(X;Y), atom(A), childItem(X,A), false(Y,A).
:- extend(X;Y), atom(A), childItem(X,a(A)), falsea(Y,A).

% A child item set cannot be extended if a removed rule is unsatisfied by it
:- extend(S), rule(X), removed(X), false(S,X).
	
% True atoms and satisfied rules remain so unless removed
item(X) :- extend(S), childItem(S,X), current(X).
item(a(X)) :- extend(S), childItem(S,a(X)), current(X).

% Guess truth value of introduced atoms
%0 { item(A;a(A)) } 1 :- atom(A), introduced(A).
{ item(A;a(A)) : atom(A), introduced(A) }.
:- item(A), item(a(A)).

% Through the guess, rules may become satisfied
item(R) :- current(R;A), head(R,A),     item(A).
item(R) :- current(R;A), pos(R,A),  not item(A).
item(R) :- current(R;A), neg(R,A),      item(A).

% If a negative body atom is true on the top level, the rule disappears from reduct (w.r.t. the top level)
item(R) :- current(R;A), neg(R,A),      item(a(A)).

auxItem(kill) :- item(a(X)), current(X).
auxItem(kill) :- extend(R), childAuxItem(R,kill).

optItem(S) :- atom(S), item(S).
optItem(a(S)) :- atom(S), item(S), not auxItem(kill).
optItem(a(S)) :- atom(S), item(a(S)).

#show optItem/1.
#show extend/1.
#show item/1.
#show auxItem/1.
