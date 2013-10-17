/*
Copyright 2012-2013, Bernhard Bliem
WWW: <http://dbai.tuwien.ac.at/research/project/dflat/>.

This file is part of D-FLAT.

D-FLAT is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

D-FLAT is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with D-FLAT. If not, see <http://www.gnu.org/licenses/>.
*/

#include <cassert>

#include "ExtensionIterator.h"

ExtensionIterator::ExtensionIterator(const ItemTreeNode& itemTreeNode, const ExtensionIterator* parent)
	: itemTreeNode(itemTreeNode)
	, parent(parent)
{
	reset();
}

bool ExtensionIterator::hasNext() const
{
	assert(valid);
	assert(curTuple != itemTreeNode.getExtensionPointers().end());

	for(const auto& it : extensionIts)
		if(it->hasNext() == true)
			return true;

	return std::next(curTuple) != itemTreeNode.getExtensionPointers().end();
}

bool ExtensionIterator::curTupleAreChildrenOfParent() const
{
	assert(parent);
	assert(curTuple != itemTreeNode.getExtensionPointers().end());
	assert(parent->curTuple != parent->itemTreeNode.getExtensionPointers().end());
	assert(curTuple->size() == parent->curTuple->size());

	ItemTreeNode::ExtensionPointerTuple::const_iterator tupleIt = curTuple->begin();
	for(const auto& parentExtension : *parent->curTuple) {
		if(tupleIt->second->getParent() != parentExtension.second.get())
			return false;
		++tupleIt;
	}
	return true;
}

bool ExtensionIterator::forwardCurTuple()
{
	assert(curTuple != itemTreeNode.getExtensionPointers().end());

	while(curTupleAreChildrenOfParent() == false) {
		++curTuple;
		if(curTuple == itemTreeNode.getExtensionPointers().end()) {
			// itemTreeNode has no extension pointer tuple such that each of its extension pointers refers to a child of parent
			valid = false;
			return false;
		}
	}
	return true;
}

void ExtensionIterator::reset()
{
	valid = true;
	curTuple = itemTreeNode.getExtensionPointers().begin();
	assert(curTuple != itemTreeNode.getExtensionPointers().end()); // If there are no decomposition children, *curTuple should be a vector of size 0
	assert(curTuple->empty() == false || extensionIts.empty()); // If the current tuple is empty, there should also be no extensionIts

	if(parent && forwardCurTuple() == false)
		return;

	extensionIts.resize(curTuple->size());
	resetExtensionPointers();

	materializeItems();
}

void ExtensionIterator::resetExtensionPointers()
{
	assert(curTuple != itemTreeNode.getExtensionPointers().end());
	assert(curTuple->size() == extensionIts.size());

	if(parent) {
		assert(curTuple->size() == parent->extensionIts.size());
		ItemTreeNode::ExtensionPointerTuple::const_iterator tupleIt = curTuple->begin();
		for(unsigned i = 0; i < curTuple->size(); ++i) {
			extensionIts[i].reset(new ExtensionIterator(*tupleIt->second, parent->extensionIts[i].get()));
			++tupleIt;
		}
	}
	else {
		ItemTreeNode::ExtensionPointerTuple::const_iterator tupleIt = curTuple->begin();
		for(unsigned i = 0; i < curTuple->size(); ++i) {
			extensionIts[i].reset(new ExtensionIterator(*tupleIt->second));
			++tupleIt;
		}
	}
}

void ExtensionIterator::materializeItems()
{
	items.clear();
	items.insert(itemTreeNode.getItems().begin(), itemTreeNode.getItems().end());
	for(const auto& it : extensionIts) {
		assert(it);
		const ItemTreeNode::Items& extension = **it;
		items.insert(extension.begin(), extension.end());
	}
}

ItemTreeNode::Items& ExtensionIterator::operator*()
{
	assert(valid);
	return items;
}

ExtensionIterator& ExtensionIterator::operator++()
{
	assert(valid);
	assert(curTuple != itemTreeNode.getExtensionPointers().end());

	incrementExtensionIterator(0);
	if(valid)
		materializeItems();

	return *this;
}

void ExtensionIterator::incrementExtensionIterator(unsigned int i) {
	if(i == extensionIts.size()) {
		// The last one was the rightmost extension iterator.
		// Use next extension pointer tuple.
		++curTuple;
		if(curTuple == itemTreeNode.getExtensionPointers().end())
			valid = false;
		else {
			if(forwardCurTuple())
				resetExtensionPointers();
		}
	}
	else {
		++(*extensionIts[i]);
		if(!extensionIts[i]->valid) {
			// Now we need to advance the next iterator left of it ("carry operation")
			extensionIts[i]->reset();
			incrementExtensionIterator(i + 1);
		}
	}
}