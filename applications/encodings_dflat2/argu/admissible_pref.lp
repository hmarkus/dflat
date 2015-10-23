%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% admissible/preferred semantics                 %
% using non-/semi-normalized tree decompositions %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

1  { extend(R)  : childRow(R,N) } 1 :- childNode(N).

0 { item(in(A)) } 1 :- introduced(A).

nIn(S,A) :- childRow(S, N), bag(N,A), not childItem(S,in(A)).
:- extend(S1), extend(S2), childItem(S1,in(A)), nIn(S2,A).

item(in(A))     :- extend(S), childItem(S,in(A)),     current(A).
auxItem(def(A)) :- extend(S), childAuxItem(S,def(A)), current(A).
auxItem(att(A)) :- extend(S), childAuxItem(S,att(A)), current(A), not auxItem(def(A)).

auxItem(def(A)) :- current(A;B), att(B,A), item(in(B)).
auxItem(att(A)) :- current(A;B), att(A,B), item(in(B)), not auxItem(def(A)).

:- extend(S), childAuxItem(S, att(A)), removed(A).
:- item(in(A)), item(in(B)), att(A,B).

optItem(A) :- item(in(A)).

#show item/1.
#show auxItem/1.
#show extend/1.
#show optItem/1.
