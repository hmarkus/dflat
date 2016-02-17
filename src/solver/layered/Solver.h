/*{{{
Copyright 2012-2013, Bernhard Bliem
WWW: <http://dbai.tuwien.ac.at/research/project/dflat/>.

This file is part of D-FLAT.

D-FLAT is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

D-FLAT is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with D-FLAT.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
//}}}
#include "../clasp/Solver.h"

namespace solver { namespace layered {

class Solver : public solver::clasp::Solver
{

	typedef enum { ESM_NONE, ESM_COMPUTE1, ESM_COMPUTE2, ESM_EXTPOINTERS } SolverMode;
public:
	Solver(const Decomposition& decomposition, const Application& app, const std::vector<std::string>& encodingFiles, bool max);

	virtual ItemTreePtr compute() override;
protected:
	::Solver* findDecompNode(unsigned int id);

	void removeExtToOldParent(ItemTreeNode& itree, unsigned int& i, mpz_class & counts);
	void insertCompressed(ItemTreeNode& itree, ItemTreeNode::SubsetNode* content, ItemTreeNode::ExtensionPointerTuple* ext, unsigned int& i, std::set<ItemTreeNode*>& forkeds);
	ItemTreeNode* addNewParentNode(ItemTreeNode& itree, ItemTreeNode::SubsetNode* content, ItemTreeNode::ExtensionPointerTuple* ext, unsigned int& i);
	
	void freeAll(ItemTreeNode* node);
	void nodeDebug(const ItemTreeNode* succ);

	void compute2();
	void calculateExtendedPointers();

	static ItemTree* superroot;
	static SolverMode mode;
	
	bool maximize;
};

}} // namespace solver::clasp
