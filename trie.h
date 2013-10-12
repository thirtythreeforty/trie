#ifndef TRIE_H
#define TRIE_H

#include <utility>
#include <vector>
#include <memory>
#include <iterator>

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
	trie();
	trie(const trie<T>&);
	trie(trie<T>&&);
	template<typename InputIt> trie(InputIt, InputIt);

	// destructors
	~trie();

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

	void swap(trie<T>);
	static void swap(trie<T>& a, trie<T>& b) { a.swap(b); }

private:
	std::vector<std::pair<typename T::value_type, std::unique_ptr<trie<T>>>> children;
	std::unique_ptr<trie<T>> parent;
};

#endif
