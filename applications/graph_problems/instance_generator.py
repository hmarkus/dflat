#!/usr/bin/python
# Generates more or less random problem instances consisting of graphs (vertex/1, edge/2)
import random
import sys

if len(sys.argv) not in (3, 4):
	sys.stderr.write("Usage: " + sys.argv[0] + " numNodes numEdges [seed]\n")
	sys.exit(1)

numNodes = int(sys.argv[1])
numEdges = int(sys.argv[2])

if len(sys.argv) == 4:
	random.seed(int(sys.argv[3]))

if numEdges < numNodes:
	sys.stderr.write("Must have at least one edge per node.\n")
	sys.exit(1)

for i in range(numNodes):
	print('vertex({}).'.format(i))

for i in range(numNodes):
	to = random.randrange(numNodes-1)
	if to >= i:
		to = to+1
	print('edge({},{}).'.format(i, to))

for i in range(numEdges - numNodes):
	source = random.randrange(numNodes)
	target = random.randrange(numNodes-1)
	if target >= source:
		target = target+1
	print('edge({},{}).'.format(source, target))
