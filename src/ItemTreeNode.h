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
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with D-FLAT.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <ostream>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <gmpxx.h>

class ItemTreeNode
{
public:
	typedef std::set<std::string> Items; // We need the sortedness for, e.g., the default join.
	typedef std::shared_ptr<ItemTreeNode> ExtensionPointer;
	typedef std::map<unsigned int, ExtensionPointer> ExtensionPointerTuple; // key: ID of the decomposition node at which value is located
	typedef std::vector<ExtensionPointerTuple> ExtensionPointers;

	enum class Type {
		UNDEFINED,
		OR,
		AND,
		ACCEPT,
		REJECT
	};

	ItemTreeNode(Items&& items = {}, ExtensionPointers&& extensionPointers = {});

	const Items& getItems() const;
	const ExtensionPointers& getExtensionPointers() const;
	const ItemTreeNode* getParent() const;
	void setParent(const ItemTreeNode*);
	const mpz_class& getCount() const;

	int getCost() const;
	void setCost(int cost);

	Type getType() const;

	// Unify extension pointers of this node with the other one's given that the item sets are equal.
	// "other" will subsequently be thrown away and only "this" will be retained.
	void merge(ItemTreeNode&& other);

	// Print this node (no newlines)
	friend std::ostream& operator<<(std::ostream& os, const ItemTreeNode& node);

private:
	Items items;
	ExtensionPointers extensionPointers;
	const ItemTreeNode* parent;
	mpz_class count; // number of possible extensions of this node
	int cost = 0;

	Type type = Type::UNDEFINED;
};

std::ostream& operator<<(std::ostream& os, const std::shared_ptr<ItemTreeNode>& node);