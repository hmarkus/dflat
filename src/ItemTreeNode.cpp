/*{{{
Copyright 2012-2016, Bernhard Bliem
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
#include <cassert>
#include <memory>
#include <limits>

#include "ItemTreeNode.h"
#include "ExtensionIterator.h"

#include <iostream>

void ItemTreeNode::refreshCount() {
if(this->extensionPointers.empty())
		count = 1;
	else {
		count = 0;
		for(const ExtensionPointerTuple& tuple : this->extensionPointers) {
			mpz_class product = 1;
			for(const auto& predecessor : tuple)
				product *= predecessor->getCount();
			count += product;
		}
	}
}


namespace {
	// Returns a negative integer if lhs < rhs, a positive integer if rhs > lhs, 0 if lhs == rhs
	int compareSets(const ItemTreeNode::Items& lhs, const ItemTreeNode::Items& rhs)
	{
		const size_t smallestSize = std::min(lhs.size(), rhs.size());
		size_t i = 0;
		for(auto it1 = lhs.begin(), it2 = rhs.begin(); i < smallestSize; ++it1, ++it2, ++i) {
			if(*it1 < *it2)
				return -1;
			else if(*it1 > *it2)
				return 1;
		}
		return lhs.size() - rhs.size();
	}
}

ItemTreeNode::ItemTreeNode(const ItemTreeNode& rhs, SubsetNode* content, ExtensionPointerTuple& ptr) 
: res(rhs.getIdentifier()) {
	this->type = rhs.getType();
	this->parent = rhs.getParent();

	//TODO: this cost stuff
	this->cost = rhs.cost;
	this->currentCost = rhs.currentCost;
	this->count = 0;//rhs.count;
	this->hasAcceptingChild = rhs.hasAcceptingChild;
	this->hasRejectingChild = rhs.hasRejectingChild;

	this->contentptr.reset(content);// = ContentPointer(new SubsetNode());
	this->extendedPointers = rhs.extendedPointers;

	(this->extensionPointers).push_back(ptr);//std::move(ptr));

	//not necessary to delete extendedPointers, because they are unique already (just one occurence within the vector)
}

bool ItemTreeNode::LessSubsetComparator::operator()(const ItemTreeNode* a, const ItemTreeNode*b)
{
        return a->getIdentifier().get() < b->getIdentifier().get();
        //return (a->getItems() < b->getItems()) || (a->getItems().size() == b->getItems().size() && !(a->getItems() > b->getItems()) && a->getAuxItems() < b->getAuxItems());
}

ItemTreeNode::ItemTreeNode(Items&& items, Items&& auxItems, ExtensionPointers&& extensionPointers, Type type, Items&& optItems)
	: res(ResourcePointer(new ItemTreeNode::ItemTreeNodeResources()))//, id.get()->items(std::move(items))
//	, auxItems(std::move(auxItems))
	, extensionPointers(std::move(extensionPointers))
	, /*extendedPointers(std::shared_ptr<ExtendedPointers>(new ExtendedPointers())),*/ contentptr(nullptr)
	, type(type)
{
	res->items = std::move(items);
	res->auxItems = std::move(auxItems);
	res->optItems = std::move(optItems);
	assert(!this->extensionPointers.empty());
	count = 0;
	for(const ExtensionPointerTuple& tuple : this->extensionPointers) {
		mpz_class product = 1;
		for(const auto& predecessor : tuple)
			product *= predecessor->getCount();
		count += product;
	}

#ifndef DISABLE_CHECKS
	for(const ExtensionPointerTuple& tuple : this->extensionPointers)
		for(const auto& predecessor : tuple)
			if(predecessor->type != Type::UNDEFINED && type != predecessor->type)
				throw std::runtime_error("Type of extended item tree node not retained");
#endif

	// Retain the information about accepting / rejecting children
	for(const ExtensionPointerTuple& tuple : this->extensionPointers) {
		for(const auto& predecessor : tuple) {
			hasAcceptingChild = hasAcceptingChild || predecessor->hasAcceptingChild;
			hasRejectingChild = hasRejectingChild || predecessor->hasRejectingChild;
		}
	}

	cost = type == Type::REJECT ? std::numeric_limits<decltype(cost)>::max() : 0;
	currentCost = 0;
}


void ItemTreeNode::addExtPointer(ItemTreeNode* ptr, unsigned int globalId)
{
	auto& m = extendedPointers[globalId];
	for (const auto& itm : m)
		if (itm == ptr)
			return;
	m.push_back(ptr);
}

/*void ItemTreeNode::removeExtPointer(ItemTreeNode* ptr, unsigned int globalId)
{
	auto xt = extendedPointers.find(globalId);
	if (xt != extendedPointers.end())
	{
		for (auto yt = xt->second.begin(); yt != xt->second.end();)
			if (*yt == ptr)
			{
				yt = xt->second.erase(yt);
				break;
			}
			else
				++yt;
	}
}*/

const ItemTreeNode::Items& ItemTreeNode::getItems() const
{
	return res->items;
}

const ItemTreeNode::Items& ItemTreeNode::getOptItems() const
{
	return res->optItems;
}

const ItemTreeNode::Items& ItemTreeNode::getAuxItems() const
{
	return res->auxItems;
}

void ItemTreeNode::setParent(const ItemTreeNode* parent)
{
	this->parent = parent;
}

const ItemTreeNode::WeakChildren& ItemTreeNode::getWeakChildren() const
{
	return weakChildren;
}
 
void ItemTreeNode::addWeakChild(const std::shared_ptr<ItemTreeNode>& child)
{
	weakChildren.emplace_back(child);
}

void ItemTreeNode::setCost(long cost)
{
#ifndef DISABLE_CHECKS
	if(type == Type::REJECT)
		throw std::runtime_error("Tried to set cost of a reject node");
#endif
	this->cost = cost;
}

void ItemTreeNode::setCurrentCost(long currentCost)
{
	this->currentCost = currentCost;
}

void ItemTreeNode::setHasAcceptingChild()
{
	hasAcceptingChild = true;
}

void ItemTreeNode::setHasRejectingChild()
{
	hasRejectingChild = true;
}

mpz_class ItemTreeNode::countExtensions(const ExtensionIterator& parentIterator) const
{
	assert(parentIterator.isValid());
	mpz_class result;
	assert(result == 0);

	if(&parentIterator.getItemTreeNode() == parent) {
		// XXX This is strikingly similar to what's happening in the constructor. What to do?
		if(extensionPointers.empty())
			result = 1;
		else {
			for(const ExtensionPointerTuple& tuple : extensionPointers) {
				mpz_class product = 1;

				assert(parentIterator.getSubIterators().size() == tuple.size());
				ExtensionIterator::SubIterators::const_iterator subIt = parentIterator.getSubIterators().begin();
				for(const auto& predecessor : tuple) {
					product *= predecessor->countExtensions(**subIt);
					++subIt;
				}
				result += product;
			}
		}
	}

	return result;
}

void ItemTreeNode::merge(ItemTreeNode&& other)
{
	assert(res->items == other.res->items);
	assert(res->auxItems == other.res->auxItems);
	assert(res->optItems == other.res->optItems);
	assert(cost == other.cost);
	assert(currentCost == other.currentCost);

	// Merge other node's data into this
	extensionPointers.insert(extensionPointers.end(), other.extensionPointers.begin(), other.extensionPointers.end());
	count += other.count;
}

int ItemTreeNode::compareCostInsensitive(const ItemTreeNode& other) const
{
	int c = compareSets(res->items, other.res->items);
	if(c != 0)
		return c;

	if(type < other.type)
		return -1;
	else if(type > other.type)
		return 1;

	if(hasAcceptingChild < other.hasAcceptingChild)
		return -1;
	else if(hasAcceptingChild > other.hasAcceptingChild)
		return 1;

	if(hasRejectingChild < other.hasRejectingChild)
		return -1;
	else if(hasRejectingChild > other.hasRejectingChild)
		return 1;

 	c = compareSets(res->auxItems, other.res->items);
	if(c != 0)
		return c;
	if (getContent() < getContent())
		return -1; //compareSets(auxItems, other.auxItems);
	else if (getContent() > getContent())
		return 1;
	return 0;
}

ItemTreeNode::Items ItemTreeNode::firstExtension() const
{
	Items result = res->items;
	assert(extensionPointers.size() > 0);
	ExtensionPointerTuple ept = extensionPointers.front();
	for(const ExtensionPointer& ep : ept) {
		const Items childResult = ep->firstExtension();
		result.insert(childResult.begin(), childResult.end());
	}
	return result;
}

std::ostream& operator<<(std::ostream& os, const ItemTreeNode& node)
{
	// Print count
//	os << '[' << node.count << "] ";

	// Print type
	switch(node.type) {
		case ItemTreeNode::Type::UNDEFINED:
			break;
		case ItemTreeNode::Type::OR:
			os << "<OR> ";
			break;
		case ItemTreeNode::Type::AND:
			os << "<AND> ";
			break;
		case ItemTreeNode::Type::ACCEPT:
			os << "<ACCEPT> ";
			break;
		case ItemTreeNode::Type::REJECT:
			os << "<REJECT> ";
			break;
	}

	if(node.hasAcceptingChild)
		os << "(a) ";
	if(node.hasRejectingChild)
		os << "(r) ";

	// Print items
	for(const auto& item : node.res->items)
		os << item << ' ';
	for(const auto& item : node.res->auxItems)
		os << item << ' ';
	for(const auto& item : node.res->optItems)
		os << item << ' ';

//	os << "; extend: {";
//	std::string tupleSep;
//	for(const auto& tuple : node.extensionPointers) {
//		os << tupleSep << '(';
//		std::string ptrSep;
//		for(const auto& extended : tuple) {
//			os << ptrSep << extended.first << ':' << extended.second.get();
//			ptrSep = ", ";
//		}
//		os << ')';
//		tupleSep = ", ";
//	}
//	os << "}, this: " << &node << ", parent: " << node.parent;

	// Print cost
	if(node.cost != 0) {
		os << " (cost: " << node.cost;
		if(node.getCurrentCost() != 0)
			os << "; current: " << node.getCurrentCost();
		os << ')';
	}

	return os;
}

std::ostream& operator<<(std::ostream& os, const std::shared_ptr<ItemTreeNode>& node)
{
	return os << *node;
}

void ItemTreeNode::setContent(SubsetNode* ptr)
{
	this->contentptr = ContentPointer(ptr);
}
