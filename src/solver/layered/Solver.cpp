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
//}}}

#include <iostream>
#include <cassert>
#include "Solver.h"
#include "../asp/Solver.h"

#include <sstream>

#include "../../Application.h"
#include "../../Printer.h"
#include "../../ItemTree.h"
#include "../../Decomposition.h"
#include "../../Application.h"

using namespace std;
using namespace solver::layered;

namespace solver { namespace layered {

Solver::Solver(const Decomposition& decomposition, const Application& app, const std::vector<std::string>& encodingFiles, bool max)
	: solver::asp::Solver(decomposition, app, encodingFiles, true), maximize(max)
{
}


#define MAINTAIN_KILL_FLAG
#define KILL_STR "kill"
//#define DEBUG
static int join = 0, total = 0, newguys = 0;

void Solver::removeExtToOldParent(ItemTreeNode& itree, unsigned int& i, mpz_class& counts)
{
	assert (itree.getExtensionPointers().size() > 1);
	auto itdel = itree.getExtensionPointers().begin() + i;
	--i;
	bool single = itree.getExtensionPointers().begin()->size() == 1;
	bool found = false;

	counts = 1;//itdel->getNode()->getCount();
	for (const auto &tpl : *itdel) 
	{
		found = true;
		counts *= tpl.second->getCount();
#ifdef DEBUG
		std::cout << "," << counts << std::endl;
#endif
		//itree.decreaseCounter(tpl.second->getCount());
		if (!single)
		{
			unsigned int cnt = 0;
			for (auto itxf = itree.getExtensionPointers().begin(); itxf != itree.getExtensionPointers().end(); ++itxf)
			{
				for (const auto &tplx : *itxf) 
					if (tplx.second == tpl.second)
					{
						++cnt;
						break;
					}
				if (cnt >= 2)
					break;
			}
			if (cnt >= 2)
				continue;
		}
		auto fnd = tpl.second->getExtPointers().find(decomposition.getNode().getGlobalId());
		assert(fnd != tpl.second->getExtPointers().end());
		if (fnd != tpl.second->getExtPointers().end()) 
		{
			auto &l = fnd->second;
			for (auto it = l.begin(); it != l.end(); ++it)
			{
				if (*it == &itree)
				{
					bool del = l.size() == 1;
					l.erase(it);
					if (del)
						tpl.second->getExtPointers().erase(fnd);
					break;
				}
			}
		}	
	}
	assert(found);
	itree.decreaseCounter(counts);
	//std::cout << "decrease " << (counts);
	assert(itree.getCount() > 0);
	itree.getExtensionPointers().erase(itdel);

	//itree.refreshCount();
}

ItemTreeNode* Solver::addNewParentNode(ItemTreeNode& itree, ItemTreeNode::SubsetNode* content, ItemTreeNode::ExtensionPointerTuple* ext, unsigned int& i)
{
	ItemTreeNode* node = nullptr;
	if (i > 0)
	{
		++solver::layered::newguys;
#ifdef DEBUG
		std::cout << "new guy" << std::endl;
#endif
		mpz_class counts;
		//removeExtToOldParent(itree, i, counts);
		
		std::shared_ptr<ItemTreeNode> dumb(new ItemTreeNode(itree, content, *ext));
		
		//TODO: procedure
		for (auto& xt : dumb->getExtensionPointers())
			for (auto &mp : xt)
				mp.second->addExtPointer(dumb.get(), decomposition.getNode().getGlobalId());

		ItemTree* p = new ItemTree(std::move(dumb));
		node = p->getNode().get();
		dumb.reset();	//WHY GOD? WHY??

		//content = p->getNode()->getContent();
		//ext = &(p->getNode()->getExtensionPointers_w()[p->getNode()->getExtensionPointers().size() - 1]);
		//mpz_class counts;
		removeExtToOldParent(itree, i, counts);
		node->increaseCounter(counts);
	
		//std::cout << "increase " << (counts);
			//add new pointer from above
		unsigned int id = (unsigned int)-1;
		for (const auto& ptr : p->getNode()->getExtPointers())
		{
			for (const auto& pt : ptr.second)
			{
				unsigned int sz = pt->getExtensionPointers().size();
				for (unsigned int q = 0; q < sz; ++q)
				{
					//bool copied = false;
					const ItemTreeNode::ExtensionPointerTuple& extss = pt->getExtensionPointers()[q];
					if (id == (unsigned int)-1)
					{
						for (const auto& tupl : extss)
							if (tupl.second.get() == &itree)
							{
								id = tupl.first;
								//copied = true;
								break;
							}
					}
					auto it = extss.find(id);
					//if (copied || ((it = extss.find(id)) != extss.end() && it->second == &itree))
					if (it != extss.end() && it->second.get() == &itree)
					{
						//changed = true;
						pt->getExtensionPointers().push_back(extss);
						pt->getExtensionPointers()[pt->getExtensionPointers().size() - 1][id] = p->getNode();
					}
				}
			}
		}

		if (itree.getExtPointers().size() == 0) 
		{
			//std::cout << "SUPERROOT" << std::endl;
			ItemTreePtr itp(p);
#ifdef OLD_VERSION
			p->getParents().push_back(superroot);
#endif
			unsigned int sz = superroot->getChildren().size();
			superroot->getChildren().insert(superroot->getChildren().end(), std::move(itp));// children.emplace(children.end(), child.get());
			assert(sz + 1 == superroot->getChildren().size());
			//superroot->getChilds().emplace(superroot->getChilds().end(), p);
			itp.release();
			//wtf.. WHY GOD? WHY? if you do not release, suddenly destructor of itp gets called.
		}

		//node->refreshCount();
	}
	return node;
}

void Solver::nodeDebug(const ItemTreeNode* succ)
{
	for (const auto& itm : succ->getItems())
			std::cout << itm << " ";
	std::cout << " | ";
	for (const auto& itm : succ->getAuxItems())
		std::cout << itm << " ";
	std::cout << " | ";
	for (const auto& itm : succ->getOptItems())
		std::cout << itm << " ";

	std::cout << " @ ";
}

//bool compareItemTreeNode(const ItemTreeNode* x, const ItemTreeNode* y) 
bool compareItemTreeNode(const std::pair<const ItemTreeNode*, bool> &x, const std::pair<const ItemTreeNode*, bool> &y) //const ItemTreeNode* x, const ItemTreeNode* y) 
//bool compareItemTreeNode(const ItemTreeNode::Items* x, const ItemTreeNode::Items* y)// const ItemTreeNode* x, const ItemTreeNode* y) 
{
	//return x == y;// || x->getIdentifier().get() == y->getIdentifier().get();
	return x.second == y.second && (x.first == y.first || x.first->getIdentifier().get() == y.first->getIdentifier().get());// || x->getIdentifier()->optItems == y->getIdentifier()->optItems;
/*  return x == y; 
	 return x == y || (x->getItems().size() == y->getItems().size() &&
    		x->getAuxItems().size() == y->getAuxItems().size() && std::equal(x->getItems().begin(), x->getItems().end(), y->getItems().begin()) && 
				std::equal(x->getAuxItems().begin(), x->getAuxItems().end(), y->getAuxItems().begin()))
*/
    /*&& std::equal(x->getAuxItems().begin(), x->getAuxItems().end(), y->getAuxItems().begin())*/;
}


void Solver::insertCompressed(ItemTreeNode& itree, ItemTreeNode::SubsetNode* content, ItemTreeNode::ExtensionPointerTuple* ext, unsigned int& i, std::set<ItemTreeNode*>& forkeds)
{
	if (i > 0)
	{
#ifdef DEBUG
		std::cout << i << ";" << &itree << "," << content << std::endl;
		if (content->size() == 0)
			std::cout << "0 SIZE" << std::endl;
#endif
		ItemTreeNode* found = nullptr;

		for (auto* c : forkeds) 
		{
			if (c->getContent()->size() ==
				content->size() && /*c->getContent()->getSubsetPointers() == content->getSubsetPointers())*/  std::equal(c->getContent()->begin(),
						c->getContent()->end(), content->begin(), compareItemTreeNode))
			{
				found = c;
				break;
			}
#ifdef DEBUG
			else if (c->getContent()->size() ==
                                content->size())
			{
				std::cout << "COMPARE_FAILED_FATAL: " << c->getContent()->size() << " vs " << content->size();
				for (const auto p: *c->getContent())
				{
					std::cout << "," << p.first << "/" << p.second << ":" << p.first->getIdentifier();
					std::cout << "(";
					nodeDebug(p.first);
					std::cout << ")";
				}
				std::cout << " VS ";
				for (const auto p: *content)
				{
					std::cout << "," << p.first << "/" << p.second << ":" << p.first->getIdentifier();
					std::cout << "(";
					nodeDebug(p.first);
					std::cout << ")";
				}
				std::cout << std::endl;
			}
#endif
		}
		if (found) 
		{
#ifdef DEBUG
			std::cout << "FOUND " << found << "," << found->getContent() << std::endl;
#endif
			//delete content;
			assert (found == &itree || itree.getExtensionPointers().size() > 1);
			if (found != &itree)// && itree.getExtensionPointers().size() > 1)
			{
#ifdef DEBUG
				std::cout << "DIFFERENT" << std::endl;
#endif
				
				found->getExtensionPointers().insert(found->getExtensionPointers().end(), itree.getExtensionPointers().begin() + i, itree.getExtensionPointers().begin() + i + 1);
				//(itree.getExtensionPointers_w().begin()+i)-> decomposition.getNode().getGlobalID()
				/*itree.getExtensionPointers_w().erase(itree.getExtensionPointers_w().begin() + i);
				--i;*/
				//BASICALLY the same as in ItemTreeNode.cpp::copycopyconstructor last lines:
				/*
					 for (auto& ptr : extensionPointers)
                				for (auto &mp : ptr)
                        				mp.second->addExtPointer(this, parentGlobalId);

				*/
				//TODO procedure?
				for (auto &mp : *ext)
					mp.second->addExtPointer(found, decomposition.getNode().getGlobalId());
				
				mpz_class counts;
				removeExtToOldParent(itree, i, counts);
				found->increaseCounter(counts);
				//node->increaseCounter(counts);
				//node->refreshCount();
				//std::cout << "increase " << counts;	
				//update flag
				/*auto it = found->getContent()->begin(), it2 = content->begin();
				for (; it != found->getContent()->end(); ++it, ++it2)
					it->second = it->second || it2->second;		*/
				//removeExtToOldParent(itree, i);

				//return true;
			}
			delete content;
			//TODO: remove?
			/*found->getContent()->getSubsetPointers().insert
				(content->getSubsetPointers().begin(), content->getSubsetPointers().end());*/
			//assert(false);
		}
		else 
		{
			auto* node = addNewParentNode(itree, content, ext, i);
			//if (!decomposition.isJoinNode())
				forkeds.insert(node);
			//return true;
			//return node;
		}
	}
	else
	{
		forkeds.insert(&itree);
	}
	//return false;
}

void Solver::freeAll(ItemTreeNode* node)
{
	for (const auto& tplkv: node->getExtensionPointers())
	{
		for (auto & ptr: tplkv)
		{
			//auto* p = ptr.second.get()->getContent();
			ptr.second->setContent(nullptr);
			//delete p;
			ptr.second->getExtPointers().clear();
		}
	}
}

void Solver::compute2()
{
	ItemTreeNode& itr = getNode();
	for (const auto & chld : itr.getExtensionPointers())
	{
		for (const auto & tpl : chld)
		{
			auto* s = findDecompNode(tpl.first);
			assert(s);	
			s->setNode(*tpl.second.get());
			s->compute();
		}
	}
	
	for(const auto& weakChild : itr.getWeakChildren()) 
	{
		if(!weakChild.expired()) 
		{
			std::shared_ptr<ItemTreeNode> sharedChild = weakChild.lock();
			auto& itree = *sharedChild.get();
			if (itree.getContent())	//return content if already calculated
			{
				continue;
			}
			else
				itree.setContent(new ItemTreeNode::SubsetNode());
#ifdef DEBUG
			std::cout << "n" << decomposition.getNode().getGlobalId() << ": ";
			nodeDebug(&itree);
			std::cout << std::endl;
#endif
			if (decomposition.getChildren().size() > 0) //itree.getExtensionPointers().size() > 0)
			{
				++solver::layered::total;
				std::set<ItemTreeNode*> forkeds;
				auto* content = itree.getContent();
				if (decomposition.isJoinNode())// itree.getExtensionPointers().size() >= 2)//join node
				{
					assert(decomposition.getChildren().size() == 2);
#ifdef DEBUG
					std::cout << "join: " << std::endl;
#endif
					++solver::layered::join;
					auto& exts = itree.getExtensionPointers();
					
					for (unsigned int i = 0; i < exts.size(); ++i)
					{
						auto* ext = &(itree.getExtensionPointers()[i]);
						if (ext->size() > 0)
						{
							if (i > 0)
								content = new ItemTreeNode::SubsetNode();
							assert(ext->size() == 2);
							auto tplkv = ext->begin(), tplkv2 = ext->begin();
							++tplkv2;
								
							freeAll(tplkv->second.get());
							freeAll(tplkv2->second.get());
							
							for (auto it = tplkv->second->getContent()->begin(); 
									it != tplkv->second->getContent()->end(); ++it )	//go downward
							{
								const auto *subs = it->first;
								//look at the results
								const auto &fnd = subs->getExtPointers().find(decomposition.getNode().getGlobalId());
								assert (fnd != subs->getExtPointers().end());

								for (auto succ: fnd->second)
								{
									//the following shice can be done way more efficiently...
									for (const auto& extis : succ->getExtensionPointers())
									{
										auto pos = extis.find(tplkv->first);
										if (pos != extis.end() && pos->second.get() == subs)
										{
											auto pos2 = extis.find(tplkv2->first);
											if (pos2 != extis.end())
											{
												for (auto itx = tplkv2->second->getContent()->begin(); 
													itx != tplkv2->second->getContent()->end(); ++itx )	//go downward
												{
													const auto *subs2 = itx->first;
													if (pos2->second.get() == subs2)
													{
														bool d;
														auto itins = content->insert(std::pair<const ItemTreeNode*, bool>(succ, d = itx->second || it->second)).first;
														itins->second = itins->second || d;

#ifdef DEBUG
														nodeDebug(pos->second.get());
														nodeDebug(pos2->second.get());
														std::cout << std::endl;
#endif
													}
												}
											}
										}
									}
								}
							}
						
							assert(ext->size() == 2);
							(insertCompressed(itree, content, ext, i, forkeds));
						}
					}
				}
				else //introduction node
				{
					auto& exts = itree.getExtensionPointers();
					for (unsigned int i = 0; i < exts.size(); ++i)
					{
						auto* ext = &(itree.getExtensionPointers()[i]);
						if (ext->size() > 0)
						{
							if (i > 0)
								content = new ItemTreeNode::SubsetNode();
							for (auto& tplkv : *ext)	//goto ext pointers
							{
								freeAll(tplkv.second.get());
								for (auto it = tplkv.second->getContent()->begin(); 
									it != tplkv.second->getContent()->end(); ++it)	//go downward
								{
									const auto *subs = it->first;
									//look at the results
									const auto &fnd = subs->getExtPointers().find(decomposition.getNode().getGlobalId());
									assert (fnd != subs->getExtPointers().end());
									
									for (auto succ: fnd->second)
									{
										if ((!maximize && (itree.getIdentifier() == succ->getIdentifier() || 
												std::includes(itree.getOptItems().begin(), itree.getOptItems().end(),
												succ->getOptItems().begin(), succ->getOptItems().end())))
								  		|| (maximize && (itree.getIdentifier() == succ->getIdentifier() || 
												std::includes(succ->getOptItems().begin(), succ->getOptItems().end(),
												itree.getOptItems().begin(), itree.getOptItems().end()))))
										{
											bool d;
											auto itins = content->insert(std::pair<const ItemTreeNode*, bool>(succ, 
												d=(maximize && succ->getOptItems().size() > itree.getOptItems().size()) || 
												(!maximize && succ->getOptItems().size() < itree.getOptItems().size()) || it->second)).first;
											itins->second = itins->second || d;
#ifdef DEBUG
											nodeDebug(succ);
											std::cout << " //" << d  << "," << succ->getOptItems().size() << "," << itree.getOptItems().size() << "//" << std::endl;
#endif
										}
									}
								}
							}
							(insertCompressed(itree, content, ext, i, forkeds));
						}
					}
				}
			}
			else	//empty (leave)
			{
				itree.getContent()->insert(std::pair<const ItemTreeNode*, bool> (&itree, false));
#ifdef DEBUG
				std::cout << "leaf: " << std::endl;
#endif
			}
			//itree.refreshCount();
		}
	}
}

::Solver* Solver::findDecompNode(unsigned int id)
{
	for (const auto & chld : decomposition.getChildren())
		if (chld->getNode().getGlobalId() == id)
			return &chld->getSolver();
	return nullptr;
}

void Solver::calculateExtendedPointers()
{
	for(const auto& weakChild : getNode().getWeakChildren()) 
	{
		if(!weakChild.expired()) 
		{
			//assert(weakChild.expired() == false);
			std::shared_ptr<ItemTreeNode> sharedChild = weakChild.lock();
			auto& itree = *sharedChild.get();
			
			for (const auto& ext : itree.getExtensionPointers())
			{
				for (const auto &t : ext)
				{
					assert(findDecompNode(t.first));
					t.second->addExtPointer(&itree, decomposition.getNode().getGlobalId());
				}
			}
		}
	}

	for (const auto& ext : getNode().getExtensionPointers())
	{
		for (const auto &t : ext)
		{
			::Solver* slv = findDecompNode(t.first);
			assert(slv);

			slv->setNode(*t.second.get());
			slv->compute(); //calculateExtendedPointers();
		}
	}

}
ItemTree* Solver::superroot = nullptr;
solver::layered::Solver::SolverMode Solver::mode = solver::layered::Solver::SolverMode::ESM_NONE;

ItemTreePtr Solver::compute()
{
	ItemTreePtr ret = nullptr;		//compute1
#ifdef OLD_VERSION
	if (decomposition.getParents().size() == 0)	//root
#else
	if (Solver::mode == ESM_NONE) //decomposition.getParents().size() == 0)	//root
#endif
	{
		assert(Solver::superroot == nullptr);	//no one fakes root!
		assert(Solver::mode == ESM_NONE);	//no one fakes root!
		Solver::mode = Solver::SolverMode::ESM_COMPUTE1;
		ret = this->solver::asp::Solver::compute();		//compute1
		//return ret;
		if (ret)
		{
			std::cerr << "FINISHED compute1" << std::endl;
			Solver::superroot = ret.get();
			Solver::mode = ESM_EXTPOINTERS;
			//calculateExtendedPointers(*(ret.get()));
			
			setNode(*(ret->getNode()));
			calculateExtendedPointers();

			std::cerr << "FINISHED extpointers" << std::endl;
			Solver::mode = solver::layered::Solver::ESM_COMPUTE2;
			setNode(*(ret->getNode()));
			this->compute2();

			std::cerr << "FINISHED compute2" << std::endl;
			bool allErased = true;
			for (auto it = ret->getChildren().begin(); it != ret->getChildren().end(); )
			{
				auto &node = *(*it)->getNode().get();
				bool found = false;
#ifdef MAINTAIN_KILL_FLAG
				if (node.getAuxItems().find(KILL_STR) != node.getAuxItems().end())
					found = true;
#endif
				if (!found && node.getContent() && node.getContent()->size() > 0)
				{
					for (const auto& el : *node.getContent())
						if (el.second)
						{
							found = true;
							break;
						}
				}

				node.setContent(nullptr);
				if (found)
				{
					//std::cout << "DECREASE " << node.getCount() << std::endl;
					//ret->getNode()->decreaseCounter(2);
					//node.setContent(nullptr);
					it = ret->getChildren().erase(it);
					
					//ret->getNode()->refreshCount();
				}
				else
				{
					allErased = false;
					++it;
				}
			}
			//ret->getNode()->refreshCount();
			std::cout << "join nodes: " << solver::layered::join << "/" << solver::layered::total << ", newguys: " << solver::layered::newguys << std::endl;
			if (allErased) {
				std::cout << "NO_SOLUTIONS" << std::endl;
				return nullptr;
			}
		}
	}
	else if (Solver::mode == Solver::ESM_EXTPOINTERS)	//level 2 reached
		this->calculateExtendedPointers();
	else if (Solver::mode == Solver::ESM_COMPUTE2)		//compute2
		this->compute2();
	else if (Solver::mode == Solver::ESM_COMPUTE1)
		ret = this->solver::asp::Solver::compute();		//compute1
	else
		assert(false);
	return ret;
}

}} // namespace solver::layered
