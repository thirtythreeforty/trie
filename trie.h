// trie.h - A templatized C++11 data container
// Copyright (C) 2013  George Hilliard
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef TRIE_H
#define TRIE_H

#include <map>
#include <stack>
#include <memory>
#include <iterator>
#include <utility>
#include <algorithm>
#include <stdexcept>
#include <limits>
#include <initializer_list>

template<typename T>
class trie {
	// data members and types
	typedef std::vector<std::pair<typename T::value_type, std::unique_ptr<trie<T>>>> child_map_type;
	child_map_type children;
	bool is_leaf = false;

public:
	// misc. declarations
	class iterator;
	typedef T key_type;
	typedef T value_type;
	typedef size_t size_type;
	typedef iterator const_iterator;

	// constructors
	trie(bool = false);
	trie(const trie<T>&);
	trie(trie<T>&&);
	template<typename InputIt> trie(InputIt, InputIt, bool = false);
	trie(std::initializer_list<T>);

	// destructor, auto-generated one is fine
	virtual ~trie() =default;

	// operators
	trie<T>& operator=(trie<T>);

	// iterators and related
	iterator begin() const;
	iterator end() const;
	const_iterator cbegin() const { return begin(); }
	const_iterator cend() const { return end(); }

	// other members
	std::pair<iterator,bool> insert(const value_type&);
	template<typename InputIt> void insert(InputIt, const InputIt&);

	iterator erase(const_iterator);
	size_type erase(const key_type&);
	iterator erase(const_iterator, const_iterator);
	void clear();

	bool empty() const { return children.empty() && !is_leaf; }
	size_type size() const;
	constexpr size_type max_size() const;

	const_iterator find(const key_type&) const;
	size_type count(const key_type&) const;

	void swap(trie<T>&);
	static void swap(trie<T>& a, trie<T>& b) { a.swap(b); }
};

// Definition of trie<T>::iterator
#include "trie_iterator.h"

template<typename T>
trie<T>::trie(bool is_leaf) :
	is_leaf{is_leaf}
{}

template<typename T>
trie<T>::trie(const trie<T>& other) :
	is_leaf{other.is_leaf}
{
	// Deep copy the children
	children.reserve(other.children.size());
	for(const auto& it : other.children) {
		// Separate creation of unique_ptr for exception safety
		std::unique_ptr<trie<T>> p(it.second == nullptr ? nullptr : new trie<T>(*it.second));
		children.emplace(children.end(), it.first, std::move(p));
	}
}

template<typename T>
trie<T>::trie(trie<T>&& other) :
	children{std::move(other.children)}, is_leaf{other.is_leaf}
{}

template<typename T>
template<typename InputIt>
trie<T>::trie(const InputIt begin, const InputIt end, bool is_leaf) :
	is_leaf{is_leaf}
{
	for(auto x = begin; x != end; ++x)
		insert(*x);
}

template<typename T>
trie<T>::trie(std::initializer_list<T> l) :
	is_leaf{false}
{
	for(const auto& e: l)
		insert(e);
}

template<typename T>
trie<T>& trie<T>::operator=(trie<T> other)
{
	swap(*this, other);
	return *this;
}

template<typename T>
auto trie<T>::begin() const -> iterator
{
	// We'll let the iterator fall down to the first valid value.
	return {this};
}

template<typename T>
auto trie<T>::end() const -> iterator
{
	// Here's where we use our friend privileges
	std::stack<typename iterator::state> temp;
	temp.push({this, this->children.cend()});
	T built;
	built.reserve(16);
	return {std::move(temp), std::move(built), true, false};
}

template<typename T>
auto trie<T>::insert(const value_type& value) -> std::pair<iterator,bool>
{
	bool inserted = false;
	iterator it;
	decltype(this) currentNode = this;

	if(value.empty()) {
		// Special case for empty value
		if(!currentNode->is_leaf) {
			inserted = true;
			currentNode->is_leaf = true;
		}
		it.parents.emplace(currentNode, currentNode->children.begin());
		it.at_leaf = true;
	}
	else
		for(auto inputIt = value.cbegin(); inputIt != value.cend(); ++inputIt) {
			bool is_last = (inputIt + 1 == value.end());

			// A clever application of <= here allows us to re-use the iterator for emplace,
			// should the requested element not be found.
			auto childIt = std::upper_bound(currentNode->children.begin(), currentNode->children.end(), *inputIt,
			                                [&inputIt](const typename T::value_type& x, const std::pair<typename T::value_type,std::unique_ptr<trie<T>>>& y)
			                                          { return x <= y.first; });
			// We must check if the iterator is at the end before trying to dereference it.
			if(childIt == currentNode->children.end() || childIt->first != *inputIt) {
				// Child is new.  Insert it with a link, to nullptr if it's the last.
				inserted = true;

				decltype(this) newtrie {is_last ? nullptr : new trie<T>};

				// FIXME Inconsistent unique_ptr construction
				it.parents.emplace(currentNode,
					currentNode->children.emplace(childIt, *inputIt, std::unique_ptr<trie<T>>(newtrie))
				);

				currentNode = newtrie;
			}
			else {
				// Child is found.  Move to it if it isn't nullptr.
				// If it is nullptr, change it to a node with a leaf flag.
				if(is_last) {
					it.parents.emplace(currentNode, childIt);
					if(childIt->second.get() != nullptr) {
						// Basically descend *twice*
						currentNode = childIt->second.get();
						if(!currentNode->is_leaf) {
							inserted = true;
							currentNode->is_leaf = true;
						}
						it.parents.emplace(currentNode, currentNode->children.begin());
						it.at_leaf = true;
					}
				}
				else {
					if(childIt->second.get() == nullptr) {
						childIt->second.reset(new trie<T>{true});
						inserted = true;
					}
					// Child now definitely exists, move to it.
					it.parents.emplace(currentNode, childIt);
					currentNode = childIt->second.get();
				}
			}
		}
	it.built = value;
	return {std::move(it),inserted};
}

template<typename T>
template<typename InputIt>
void trie<T>::insert(InputIt start, const InputIt& finish)
{
	while(start != finish)
		insert(*(start++));
}

template<typename T>
auto trie<T>::erase(const_iterator it) -> iterator
{
	auto nextit(it);
	++nextit;

	// The const_cast<>s here are actually not bad design.
	// The reason for this is that the iterator must be an iterator of *this,
	// or behavior is undefined.  Because this non-const function is executing,
	// it means that the pointer must be to a non-const trie<T>!

	if(it.at_leaf)
		const_cast<trie<T>*&>(it.parents.top().node)->is_leaf = false;
	else {
		while(!it.parents.top().node->is_leaf && it.parents.top().node->children.size() == 1 && it.parents.size() > 1)
			it.parents.pop();

		// Because GCC 4.8.x does not implement the C++11 function with signature
		//    auto std::vector<T>::erase(const_iterator) -> iterator
		// You may required to use this comparatively nasty (and slower) construct:
		//auto nonconst_it = const_cast<trie<T>*&>(it.parents.top().node)->children.begin();
		//std::advance(nonconst_it, std::distance(it.parents.top().node->children.cbegin(), it.parents.top().node_map_it));
		//const_cast<trie<T>*&>(it.parents.top().node)->children.erase(nonconst_it);
		const_cast<trie<T>*&>(it.parents.top().node)->children.erase(it.parents.top().node_map_it);

		if(it.parents.top().node->children.size() == 0 && it.parents.size() > 1) {
			it.parents.pop();
			const_cast<std::unique_ptr<trie<T>>&>(it.parents.top().node_map_it->second).reset(nullptr);
		}
	}

	// Because the child list is a vector, we must re-find the next value, because iterators have been invalidated.
	// It doesn't need to be recreated if the iterator is at_end.
	return nextit.at_end ? end() : find(*nextit);
}

template<typename T>
auto trie<T>::erase(const key_type& key) -> size_type
{
	auto found = find(key);
	if(found == end())
		return 0;
	else {
		erase(found);
		return 1;
	}
}

template<typename T>
auto trie<T>::erase(const_iterator first, const_iterator last) -> iterator
{
	while(first != last)
		first = erase(first);

	return last;
}

template<typename T>
void trie<T>::clear()
{
	is_leaf = false;
	children.clear();
}

template<typename T>
auto trie<T>::size() const -> size_type
{
	size_type s = is_leaf ? 1 : 0;
	for(const auto& child : children)
		if(child.second != nullptr)
			s += child.second->size();
		else
			++s;
	return s;
}

template<typename T>
auto trie<T>::find(const key_type& key) const -> const_iterator
{
	iterator it{std::stack<typename iterator::state>{}, key, false, false};
	decltype(this) currentNode{this};

	if(key.empty()) {
		// Special case for empty value
		if(!this->is_leaf)
			return cend();
		else {
			it.parents.emplace(currentNode, currentNode->children.cbegin());
			it.at_leaf = true;
		}
	}
	else
		for(auto inputIt = key.cbegin(); inputIt != key.cend(); ++inputIt) {
			auto childIt = std::upper_bound(currentNode->children.cbegin(), currentNode->children.cend(), *inputIt,
			                                [&inputIt](const typename T::value_type& x, const std::pair<typename T::value_type,std::unique_ptr<trie<T>>>& y)
			                                          { return x <= y.first; });
			if(childIt == currentNode->children.end() || childIt->first != *inputIt)
				// Child is not found
				return cend();
			else {
				// Child is found.  Move to it.
				bool is_last = (inputIt + 1 == key.end());
				if(is_last) {
					it.parents.emplace(currentNode, childIt);
					if(childIt->second.get() != nullptr) {
						// Basically descend *twice*
						currentNode = childIt->second.get();
						if(!currentNode->is_leaf)
							return cend();
						else {
							it.parents.emplace(currentNode, currentNode->children.begin());
							it.at_leaf = true;
						}
					}
				}
				else {
					if(childIt->second.get() == nullptr)
						return cend();
					// Child now definitely exists, move to it.
					it.parents.emplace(currentNode, childIt);
					currentNode = childIt->second.get();
				}
			}
		}
	return it;
}

template<typename T>
auto trie<T>::count(const key_type& key) const -> size_type
{
	return find(key) == cend() ? 0 : 1;
}

template<typename T>
constexpr auto trie<T>::max_size() const -> size_type
{
	// We have a depth limited only by the size of the iterator stack,
	// and a width limited by the size of the size of the vector.
	// Multiplying these numbers will overflow size_type.
	return std::numeric_limits< size_type >::max();
}

template<typename T>
void trie<T>::swap(trie<T>& other)
{
	std::swap(children, other.children);
	std::swap(is_leaf, other.is_leaf);
}

#endif
