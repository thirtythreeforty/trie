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
	trie(trie<T>* const = nullptr, bool = false);
	trie(const trie<T>&, trie<T>* const = nullptr);
	trie(trie<T>&&);
	template<typename InputIt> trie(InputIt, InputIt, trie<T>* const = nullptr);

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

	template<class... Args>
	std::pair<iterator,bool> emplace(Args&&...);

	iterator erase(const_iterator);
	size_type erase(const key_type&);
	iterator erase(const_iterator, const_iterator);
	void clear();

	bool empty() const { return children.empty(); }
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
	trie<T>* parent;
	bool is_leaf = false;
};

// Definition of trie<T>::iterator
#include "trie_iterator.h"

template<typename T>
trie<T>::trie(trie<T>* const parent, bool is_leaf) :
	parent{parent}, is_leaf{is_leaf}
{}

template<typename T>
trie<T>::trie(const trie<T>& other, trie<T>* const parent) :
	parent{parent}, is_leaf{other.is_leaf}
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
	parent{other.parent}, children{std::move(other.children)}, is_leaf{other.is_leaf}
{}

template<typename T>
template<typename InputIt>
trie<T>::trie(InputIt begin, InputIt end, trie<T>* const parent) :
	parent{parent}
{
	std::for_each(std::move(begin), std::move(end), this->insert);
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
	bool at_leaf = false;
	std::stack<typename iterator::state> parents;
	trie<T>* currentNode{this};

	for(auto it = value.cbegin(); it != value.cend(); ++it) {
		bool is_last = (it + 1 == value.end());
		auto childIt = currentNode->children.find(*it);
		if(childIt == currentNode->children.end()) {
			inserted = true;
			if(is_last) {
				// The sequence is new to this trie, so insert it.
				// It is the last element, so don't create a new trie.
				parents.emplace(
					currentNode,
					currentNode->children.emplace(std::make_pair(*it, std::unique_ptr<trie<T>>(nullptr))).first
				);
			}
			else {
				// Create a new trie and follow it.
				std::unique_ptr<trie<T>> p(new trie<T>(currentNode));
				currentNode = currentNode->children.emplace(*it, std::move(p)).first->second.get();
			}
		}
		else {
			if(is_last) {
				if(childIt->second != nullptr) {
					inserted = true;
					at_leaf = true;
					childIt->second->is_leaf = true;
				}
				// Done.  Build a return value.
				// TODO
			}
			else {
				if(childIt->second == nullptr) {
					childIt->second.reset(new trie<T>(currentNode));
					inserted = true;
				}
				currentNode = childIt->second.get();
			}
		}
	}
	// Build pair and return it
	return {{std::move(parents), value, at_leaf, false}, inserted};
}

#endif
