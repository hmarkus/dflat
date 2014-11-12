/*{{{
Copyright 2012-2014, Bernhard Bliem
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
#include <ostream>
#include <vector>
#include <set>
#include <map>
#include <memory>
#include <string>
#include <gmpxx.h>
#include <unordered_map>
#include <iostream>
//#include "SubsetNode.h"

class ExtensionIterator;

// IMPORTANT NOTE: Remember that when you change something here it might
// require changing other classes like ItemTreePtrComparator (and
// solver::asp::trees::UncompressedItemTreePtrComparator).

/*template <class T, class D>
class shared_ptrex : public std::shared_ptr<T>
{
	public:

		inline D getData() const { return m_subset; }
		inline void setData(const D s) { m_subset = s; }

		inline shared_ptrex(const std::shared_ptr<T> &i) : std::shared_ptr<T>(i), m_subset(0) { }
		inline shared_ptrex(const std::weak_ptr<T> &i) : std::shared_ptr<T>(i), m_subset(0) { }
		inline shared_ptrex(T* element) : std::shared_ptr<T>(element), m_subset(0) { }

		inline shared_ptrex() : shared_ptrex<T, D>(nullptr) { }

		inline virtual ~shared_ptrex() { this->reset(); }

		inline void operator=(const shared_ptrex<T, D>& other) { std::shared_ptr<T>::operator=(other); m_subset = other.m_subset; }
		inline void operator=(const shared_ptrex<T, D>&& other) { std::shared_ptr<T>::operator=(other); m_subset = other.m_subset; }

		inline void operator=(const std::shared_ptr<T>& other) { std::shared_ptr<T>::operator=(other); m_subset = 0; }
		inline void operator=(const std::shared_ptr<T>&& other) { std::shared_ptr<T>::operator=(other); m_subset = 0; }

		inline void operator=(const T* other) { std::shared_ptr<T>::operator=(other); }

		inline shared_ptrex(const shared_ptrex<T, D>& other) : std::shared_ptr<T>(other) { m_subset = other.m_subset; }
		inline shared_ptrex(const shared_ptrex<T, D>&& other) : std::shared_ptr<T>(other) { m_subset = other.m_subset; }

	protected:
		D m_subset;
};*/


class ItemTreeNode
{
public:
	typedef std::set<std::string> Items; // We need the sortedness for, e.g., the default join.
	//typedef shared_ptrex<ItemTreeNode, bool> ExtensionPointer;
	typedef std::shared_ptr<ItemTreeNode> ExtensionPointer;
	typedef std::map<unsigned int, ExtensionPointer> ExtensionPointerTuple; // key: ID of the decomposition node at which value is located
	typedef std::vector<ExtensionPointerTuple> ExtensionPointers;



	
	typedef std::unordered_map<unsigned int, std::vector<ItemTreeNode *>> ExtendedPointers;
 	typedef std::vector<std::weak_ptr<ItemTreeNode>> WeakChildren;
	
	struct LessSubsetComparator
	{
		bool operator()(const ItemTreeNode*, const ItemTreeNode*);
	};

	typedef std::map<const ItemTreeNode *, bool, LessSubsetComparator> SubsetNode;
	//typedef std::set<const Items*, LessSubsetComparator> SubsetNode;

	struct ItemTreeNodeResources
	{
		Items items;
		Items optItems;
		Items auxItems;
	};
	typedef std::unique_ptr<SubsetNode> ContentPointer;
	typedef std::shared_ptr<ItemTreeNodeResources> ResourcePointer;




	enum class Type {
		UNDEFINED,
		OR,
		AND,
		ACCEPT,
		REJECT
	};

	// extensionPointers may not be empty; if there are no decomposition
	// children, it should contain a 0-tuple.
	// Sets the count to the sum of the products of the counts for each
	// extension pointer tuple.
	// Sets hasAcceptingChild and hasRejectingChild to true if any extended
	// node has set the respective flag to true.
	// Unless checks are disabled, throws an exception if any extended node has
	// a defined type and the given node type is different from this.
	// If the type is REJECT, sets the cost to the highest possible value,
	// otherwise to 0.
	ItemTreeNode(Items&& items = {}, Items&& auxItems = {}, ExtensionPointers&& extensionPointers = {{}}, Type type = Type::UNDEFINED, Items&& optItems= {});

	// Returns the items of this node (but not the auxiliary items, see below).
	const Items& getItems() const;

	// Returns the items that have been declared as auxiliary.
	// These items are disregarded in the default join.
	const Items& getAuxItems() const;
	const Items& getOptItems() const;

	const ExtensionPointers& getExtensionPointers() const;
	void clearExtensionPointers();



	//"copy" constructor
	ItemTreeNode(const ItemTreeNode& rhs, SubsetNode* content, ExtensionPointerTuple& pt);
	inline ExtensionPointers& getExtensionPointers() { return extensionPointers; }
	void addExtPointer(ItemTreeNode* ptr, unsigned int globalId);
	//void removeExtPointer(ItemTreeNode* ptr, unsigned int globalId);
	void refreshCount();
	
	inline const ExtendedPointers& getExtPointers() const { return extendedPointers; }
	inline ExtendedPointers& getExtPointers() { return extendedPointers; }

	const WeakChildren& getWeakChildren() const;
 	// Constructs a weak_ptr from the argument and adds it to the list of weak children.
 	void addWeakChild(const std::shared_ptr<ItemTreeNode>& child);
	
	void setContent(SubsetNode* ptr);
	inline SubsetNode* getContent() const { return contentptr.get(); }
	const inline ResourcePointer& getIdentifier() const { return res; }
	//inline void decreaseCounter(unsigned int nr) { count -= nr; }
	inline void decreaseCounter(mpz_class& nr) { count -= nr; }
	//inline void increaseCounter(unsigned int nr) { count += nr; }
	inline void increaseCounter(mpz_class& nr) { count += nr; }
	//inline void setCounter(mpz_class& nr) { count = nr; }

		
	const ItemTreeNode* getParent() const;
	void setParent(const ItemTreeNode*);

	const mpz_class& getCount() const;

	long getCost() const;
	// Throws a runtime_error if this is a REJECT node.
	void setCost(long cost);

	long getCurrentCost() const;
	void setCurrentCost(long currentCost);

	Type getType() const;

	bool getHasAcceptingChild() const;
	void setHasAcceptingChild();
	bool getHasRejectingChild() const;
	void setHasRejectingChild();

	// Calculate the number of extensions of this node given an iterator pointing to an extension of this node's parent.
	// This is different from getCount() since getCount() returns the number of extensions for *any* possible extension of the parent.
	// This method traverses the entire decomposition.
	// If parentIterator does not point to this node's parent, returns 0.
	// parentIterator must be valid.
	mpz_class countExtensions(const ExtensionIterator& parentIterator) const;

	// Unify extension pointers of this node with the other one's given that the item sets are equal.
	// "other" will subsequently be thrown away and only "this" will be retained.
	void merge(ItemTreeNode&& other);

	// Returns true if this is "smaller" than other, without considering costs.
	// Only considers items, type, hasAcceptingChild, hasRejectingChild and auxItems.
	bool compareCostInsensitive(const ItemTreeNode& other) const;

	// Print this node (no newlines)
	friend std::ostream& operator<<(std::ostream& os, const ItemTreeNode& node);

	//virtual~ ItemTreeNode() { std::cout << "nodedestr" << std::endl; }

	   //virtual ~ItemTree() { std::cout << "destr" << std::endl; }
private:


	//const ItemTreeNode* id;
	//Items items;
	//Items auxItems;
	ResourcePointer res;
	ExtensionPointers extensionPointers;
	ExtendedPointers extendedPointers;
	ContentPointer contentptr;
	WeakChildren weakChildren;


	const ItemTreeNode* parent = nullptr;
	//bool strictsubset;
	mpz_class count; // number of possible extensions of this node
	long cost = 0;
	long currentCost = 0;
	Type type;
	// Whether this node has a child whose acceptance status (either due to ACCEPT/REJECT in leaves or propagation in AND/OR nodes) is ACCEPT or REJECT, respectively.
	// We must keep track of this explicitly since accepting/rejecting children might have been pruned.
	bool hasAcceptingChild = false;
	bool hasRejectingChild = false;
};

std::ostream& operator<<(std::ostream& os, const std::shared_ptr<ItemTreeNode>& node);
