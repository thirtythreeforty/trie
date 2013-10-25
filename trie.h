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
	trie<T>& operator=(trie<T>);

	// iterators and related
	iterator begin() const;
	iterator end() const;
	const_iterator cbegin() const { return begin(); }
	const_iterator cend() const { return end(); }

	// other members
	std::pair<iterator,bool> insert(const value_type&);

	iterator erase(const_iterator);
	size_type erase(const key_type&);
	iterator erase(const_iterator, const_iterator);
	void clear();

	bool empty() const { return children.empty() && !is_leaf; }
	size_type size() const;
	constexpr size_type max_size() const;

	const_iterator find(const key_type&) const;
	size_type count(const key_type&) const;

	const_iterator lower_bound(const value_type&) const;
	const_iterator upper_bound(const value_type&) const;
	std::pair<const_iterator,const_iterator> equal_range(const value_type&) const;

	void swap(trie<T>&);
	static void swap(trie<T>& a, trie<T>& b) { a.swap(b); }

private:
	typedef std::vector<std::pair<typename T::value_type, std::unique_ptr<trie<T>>>> child_map_type;
	child_map_type children;
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
	for(const auto& it : other.children) {
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
	for(const auto& x = begin; x != end; ++x)
		insert(*x);
}

template<typename T>
trie<T>& trie<T>::operator=(trie<T> other)
{
	swap(*this, other);
	return *this;
}

template<typename T>
typename trie<T>::iterator trie<T>::begin() const
{
	// We'll let the iterator fall down to the first valid value.
	return {this};
}

template<typename T>
typename trie<T>::iterator trie<T>::end() const
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
typename trie<T>::iterator trie<T>::erase(const_iterator it)
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
		while(!it.parents.top().node->is_leaf && it.parents.top().node->children.size() == 1)
			it.parents.pop();

		// HACK! Because GCC 4.8.x does not implement the C++11 function with signature
		//    auto std::vector<T>::erase(const_iterator) -> iterator
		// I am required to reinterpret_cast<> a pointer to the const_iterator to be a pointer
		// to a regular iterator.  I have no guarantee that this will work, and as soon as GCC
		// 4.9 is released, this should be replaced with:
		//const_cast<trie<T>*&>(it.parents.top().node)->children.erase(it.parents.top().node_map_it);
		const_cast<trie<T>*&>(it.parents.top().node)->children.erase(*reinterpret_cast<typename child_map_type::iterator*>(&(it.parents.top().node_map_it)));
	}

	return nextit;
}

template<typename T>
typename trie<T>::iterator trie<T>::erase(const_iterator first, const_iterator last)
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
typename trie<T>::size_type trie<T>::size() const
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
typename trie<T>::const_iterator trie<T>::find(const key_type& key) const
{
	iterator it{{}, key, false, false};
	decltype(this) currentNode{this};

	if(key.empty()) {
		// Special case for empty value
		if(!this->is_leaf)
			it = cend();
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
				it = cend();
			else {
				// Child is found.  Move to it.
				bool is_last = (inputIt + 1 == key.end());
				if(is_last) {
					it.parents.emplace(currentNode, childIt);
					if(childIt->second.get() != nullptr) {
						// Basically descend *twice*
						currentNode = childIt->second.get();
						if(!currentNode->is_leaf)
							it = cend();
						else {
							it.parents.emplace(currentNode, currentNode->children.begin());
							it.at_leaf = true;
						}
					}
				}
				else {
					if(childIt->second.get() == nullptr)
						it = cend();
					// Child now definitely exists, move to it.
					it.parents.emplace(currentNode, childIt);
					currentNode = childIt->second.get();
				}
			}
		}
	return it;
}

template<typename T>
typename trie<T>::size_type trie<T>::count(const key_type& key) const
{
	return find(key) == cend() ? 0 : 1;
}

template<typename T>
constexpr typename trie<T>::size_type trie<T>::max_size() const
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
