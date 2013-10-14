#ifndef TRIE_H
#define TRIE_H

#include <map>
#include <memory>
#include <iterator>
#include <utility>

template<typename T>
class trie {
public:
	// misc. declarations
	class iterator {
	public:
		const T operator*() const;
		void operator++();
		void operator--();
	};
	typedef T key_type;
	typedef T value_type;
	typedef size_t size_type;
	typedef iterator const_iterator;

	// constructors
	trie(trie<T>* const = nullptr);
	trie(const trie<T>&, trie<T>* const = nullptr);
	trie(trie<T>&&);
	template<typename InputIt> trie(InputIt, InputIt);

	// destructor, auto-generated one is fine
	// ~trie();

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
	std::pair<iterator,bool> insert(value_type&& val);

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
};

template<typename T>
trie<T>::trie(trie<T>* const parent) :
	parent{parent}
{}

template<typename T>
trie<T>::trie(const trie<T>& other, trie<T>* const parent) :
	parent{parent}
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
	parent{other.parent}, children{std::move(other.children)}
{}

#endif
