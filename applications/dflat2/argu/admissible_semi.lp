%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% admissible semantics                    %
% using nonnormalized tree decompositions %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

1  { extend(R)  : childRow(R,N) } 1 :- childNode(N).
0 { item(in(A)); defc(A) } 1 :- introduced(A).

item(in(A))     :- extend(S), childItem(S, in(A)),     current(A).

childdefc(A) :- extend(S), childAuxItem(S, defc(A)), current(A).
 childdef(A) :- extend(S), childAuxItem(S, def(A)), current(A).

def(A) :- current(A;B), att(B,A), item(in(B)), not item(in(A)).
auxItem(def(A)) :- childdef(A).
auxItem(def(A)) :- def(A).

auxItem(defc(A)) :- childdefc(A), not auxItem(def(A)).
auxItem(defc(A)) :- defc(A), not auxItem(def(A)).

:- def(A), not auxItem(defc(A)), not defc(A), not childdefc(A), not childdef(A).
:- not auxItem(defc(A)), not defc(A), not childdefc(A), not childdef(A), current(A), item(in(B)), att(A,B).

#show item/1.
#show auxItem/1.
#show extend/1.
#show optItem/1.

notIn(S,A) :- childRow(S, N), bag(N,A), not childItem(S,in(A)).
notDef(S,A) :- childRow(S, N), bag(N,A), not childAuxItem(S, def(A)), not childAuxItem(S, defc(A)).

:- extend(S1), extend(S2), childItem(S1,in(A)), notIn(S2,A).
:- extend(S1), extend(S2), childAuxItem(S1,def(A)), notDef(S2,A).
:- extend(S1), extend(S2), childAuxItem(S1,defc(A)), notDef(S2,A).


:- item(in(A)), item(in(B)), att(A,B).
:- extend(S), childAuxItem(S, defc(A)), removed(A).

optItem(A) :- item(in(A)).
optItem(A) :- auxItem(defc(A)).
optItem(A) :- auxItem(def(A)).


%auxItem(att(A)) :- extend(S), childAuxItem(S, att(A)), not auxItem(def(A)).
%auxItem(att(A)) :- current(A;B), att(A,B), item(in(B)), not item(in(A)), not auxItem(def(A)).
%:- extend(S), childAuxItem(S, att(A)), removed(A).
