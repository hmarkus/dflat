%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% admissible semantics                    %
% using nonnormalized tree decompositions %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

1  { extend(R)  : childRow(R,N) } 1 :- childNode(N).
%guess in or def
0 { item(in(A)); defc(A) } 1 :- introduced(A).

%inherit in
item(in(A))     :- extend(S), childItem(S, in(A)),     current(A).

%childs
childdefc(A) :- extend(S), childAuxItem(S, defc(A)), current(A).
childdef(A) :- extend(S), childAuxItem(S, def(A)), current(A).

%true def 
def(A) :- current(A;B), att(B,A), item(in(B)), not item(in(A)).
auxItem(def(A)) :- childdef(A).
auxItem(def(A)) :- def(A).

%inherit defc/set defc for the first time
auxItem(defc(A)) :- childdefc(A), not auxItem(def(A)).
auxItem(defc(A)) :- defc(A), not auxItem(def(A)).

%out is not allowed to be def!
out(A) :- not defc(A), not childdefc(A), not childdef(A), current(A).
:- def(A), out(A).
:- not auxItem(defc(A)), out(A), current(A), item(in(B)), att(A,B).

%join only same stuff
notIn(S,A) :- childRow(S, N), bag(N,A), not childItem(S,in(A)).
notDef(S,A) :- childRow(S, N), bag(N,A), not childAuxItem(S, def(A)), not childAuxItem(S, defc(A)).

:- extend(S1), extend(S2), childItem(S1,in(A)), notIn(S2,A).
:- extend(S1), extend(S2), childAuxItem(S1,def(A)), notDef(S2,A).
:- extend(S1), extend(S2), childAuxItem(S1,defc(A)), notDef(S2,A).

%kill in with in
:- item(in(A)), item(in(B)), att(A,B).

%kill candidates, which were only candidates
:- extend(S), childAuxItem(S, defc(A)), removed(A).

%opt stuff
optItem(A) :- item(in(A)).
optItem(A) :- auxItem(defc(A)).
optItem(A) :- auxItem(def(A)).

#show item/1.
#show auxItem/1.
#show extend/1.
#show optItem/1.


