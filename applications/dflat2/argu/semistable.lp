%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% semi-stable semantics                   %
% using nonnormalized tree decompositions %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

length(2).
or(0).
and(1).

1 #count { R  : extend(0,R)  : rootOf(R,N) } 1 :- childNode(N).
1 #count { S1 : extend(1,S1) : sub(R,S1)   } 1 :- extend(0,R).
1 #count { S2 : extend(2,S2) : sub(S1,S2)  } 1 :- extend(1,S1).

%ddddnotIn(S,A) :- childNode(N), atNode(S,N), not atLevel(S,0), bag(N,A), arg(A), not childItem(S,in(A)).

0 { item(1,in(A)); defc(1, A) } 1 :- introduced(A).
0 { item(2,in(A)); defc(2, A) } 1 :- introduced(A).

s(L, A) :- item(L, in(A)).
s(L, A) :- auxItem(L, defc(A)).
s(L, A) :- auxItem(L, def(A)).

:- s(1, A), not s(2, A).

item(L,in(A))     :- extend(L,S), childItem(S, in(A)),     current(A).

:- item(L,in(A)), item(L,in(B)), att(A,B).

childdefc(L, A) :- extend(L, S), childAuxItem(S, defc(A)), current(A).
childdef(L, A)  :- extend(L, S), childAuxItem(S, def(A)), current(A).

def(L, A) :- current(A;B), att(B,A), item(L, in(B)), not item(L, in(A)).
auxItem(L, def(A)) :- childdef(L, A).
auxItem(L, def(A)) :- def(L, A).

auxItem(L, defc(A)) :- childdefc(L, A), not auxItem(L, def(A)).
auxItem(L, defc(A)) :- defc(L, A), not auxItem(L, def(A)).

:- def(L, A), not auxItem(L, defc(A)), not defc(L, A), not childdefc(L, A), not childdef(L, A).
:- not auxItem(L, defc(A)), not defc(L, A), not childdefc(L, A), not childdef(L, A), current(A), item(L, in(B)), att(A,B).

%:- item(L, in(A)), not s(L, B), arg(B), att(B, A), current(B).

notIn(S,A) :- childNode(N), atNode(S,N), bag(N,A), not childItem(S,in(A)).
notDef(S,A) :- childNode(N), atNode(S,N), bag(N,A), not childAuxItem(S, def(A)), not childAuxItem(S, defc(A)).

:- extend(L, S1), extend(L, S2), childItem(S1,in(A)), notIn(S2,A).
:- extend(L, S1), extend(L, S2), childAuxItem(S1,def(A)), notDef(S2,A).
:- extend(L, S1), extend(L, S2), childAuxItem(S1,defc(A)), notDef(S2,A).

:- extend(L, S), childAuxItem(S, defc(A)), removed(A).

auxItem(2,cert) :- extend(2,S), childAuxItem(S,cert).	%inherit strict
auxItem(2,cert) :- s(2, A), not s(1, A).	%bigger S

reject :- final, auxItem(2,cert).
accept :- final, not reject.

#show length/1.
#show or/1.
#show and/1.
#show reject/0.
#show accept/0.
#show item/2.
#show auxItem/2.
#show extend/2.

