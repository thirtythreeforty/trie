#ifndef TRIE_H
#define TRIE_H

#include <map>
#include <stack>
#include <memory>
#include <iterator>
#include <utility>
#include <algorithm>
#include <stdexcept>

template<typename T>
class trie {
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

	// destructor, auto-generated one is fine
	~trie() =default;

	// operators
	trie<T>& operator=(const trie<T>&);

	// iterators and related
	iterator begin();
	iterator end();
	std::reverse_iterator<iterator> rbegin();
	std::reverse_iterator<iterator> rend();
	const_iterator cbegin() const;
	const_iterator cend() const;
	std::reverse_iterator<const_iterator> crbegin() const;
	std::reverse_iterator<const_iterator> crend() const;

	// other members
	std::pair<iterator,bool> insert(const value_type&);

	iterator erase(const_iterator);
	size_type erase(const key_type&);
	iterator erase(const_iterator, const_iterator);
	void clear();

	bool empty() const { return children.empty() && !is_leaf; }
	size_type size() const;
	size_type max_size() const;

	const_iterator find(const key_type&) const;
	size_type count(const key_type&) const;

	const_iterator lower_bound(const value_type&) const;
	const_iterator upper_bound(const value_type&) const;
	std::pair<const_iterator,const_iterator> equal_range(const value_type&) const;

	void swap(trie<T>&);
	static void swap(trie<T>& a, trie<T>& b) { a.swap(b); }

private:
	std::map<typename T::value_type, std::unique_ptr<trie<T>>> children;
	bool is_leaf = false;
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
	// Protip:  change unique_ptr to smart_ptr and implement a copy-on-write
	// performance boost

	// Deep copy the children
	for(auto const &it : other.children) {
		// Separate creation of unique_ptr for exception safety
		std::unique_ptr<trie<T>> p(new trie<T>(*it.second));
		children.emplace(it.first, std::move(p));
	}
}

template<typename T>
trie<T>::trie(trie<T>&& other) :
	children{std::move(other.children)}, is_leaf{other.is_leaf}
{}

template<typename T>
template<typename InputIt>
trie<T>::trie(InputIt begin, InputIt end, bool is_leaf) :
	is_leaf{is_leaf}
{
	for(auto x = begin; x != end; ++x)
		insert(*x);
}

template<typename T>
trie<T>& trie<T>::operator=(const trie<T>& other)
{
	swap(*this, other);
	return *this;
}

template<typename T>
typename trie<T>::iterator trie<T>::begin()
{
	return {this};
}

template<typename T>
typename trie<T>::iterator trie<T>::end()
{
	// Here's where we use our friend privileges
	std::stack<typename iterator::state> temp;
	temp.push({this, this->children.cend()});
	return {std::move(temp), {}, true, false};
}

template<typename T>
std::pair<typename trie<T>::iterator,bool> trie<T>::insert(const value_type& value)
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

			auto childIt = currentNode->children.find(*inputIt);
			if(childIt == currentNode->children.end()) {
				// Child is new.  Insert it with a link, to nullptr if it's the last.
				inserted = true;

				decltype(this) newtrie {is_last ? nullptr : new trie<T>};
				std::unique_ptr<trie<T>> newtrie_u_p(newtrie);

				auto insertedIt = currentNode->children.emplace(*inputIt, std::move(newtrie_u_p)).first;
				it.parents.emplace(currentNode, std::move(insertedIt));

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
void trie<T>::clear()
{
	is_leaf = false;
	children.clear();
}

#endif
