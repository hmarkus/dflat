{ in(X) : vertex(X) }.
:- edge(X,Y), not in(X;Y).
#minimize{ 1,X : in(X) }.

#show in/1.
