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
	class iterator {
		friend class trie<T>;
		// TODO: Either this structure or trie::parent is not needed
		struct state {
			state(const trie<T>* const node, const typename std::map<typename T::value_type, std::unique_ptr<trie<T>>>::const_iterator& node_map_it ) :
				node{node}, node_map_it{node_map_it} {}
			bool operator==(const state& other) const {
				return node == other.node && node_map_it == other.node_map_it;
			}
			const trie<T>* node;
			typename std::map<typename T::value_type, std::unique_ptr<trie<T>>>::const_iterator node_map_it;
		};
	public:
		iterator() =default;
		iterator(trie<T>* node) {
			parents.emplace(node, node->children.cbegin());
			fall_down();
		}
		~iterator() =default;
		iterator(typename trie<T>::iterator& other) =default;
		iterator(typename trie<T>::iterator&& other) :
			parents{std::move(other.parents)},
			built{std::move(other.built)}
		{}
		iterator& operator=(typename trie<T>::iterator other) {
			swap(*this, other);
			return *this;
		}

		void swap(typename trie<T>::iterator& other) {
			std::swap(parents, other.parents);
			std::swap(built, other.built);
		}
		static void swap(typename trie<T>::iterator& a, typename trie<T>::iterator& b) { a.swap(b); }

		const T& operator*() const { return built; }
		const T* operator->() const { return &built; }

		void operator++() {
			while(++parents.top().node_map_it == parents.top().node->children.cend())
				walk_up();
			fall_down();
		}
		void operator--() {
			while(parents.top().node_map_it-- == parents.top().node->children.cbegin())
				walk_up();
			fall_down();
		}

		bool operator==(const typename trie<T>::iterator& other) const {
			return parents.top() == other.parents.top() && at_end == other.at_end;
		}
		bool operator!=(const typename trie<T>::iterator& other) const { return !operator==(other); }
	private:
		void inline fall_down() {
			// TODO: This function could possibly be made smaller.
			trie<T>* child;
			while((child = parents.top().node_map_it->second.get()) != nullptr) {
				built.push_back(parents.top().node_map_it->first);
				parents.emplace(child, child->children.cbegin());
			}
			// One final push_back to put the final element (the one that has no
			// children) in built.
			built.push_back(parents.top().node_map_it->first);
		}
		void inline walk_up() {
			built.pop_back();
			parents.pop();
		}

		std::stack<state> parents;
		// TODO: we could switch the use of push_back and pop_back for insert and erase
		// using an end iterator, to gain some additional compatibility.
		T built;
		bool at_end = false;
	};
	typedef T key_type;
	typedef T value_type;
	typedef size_t size_type;
	typedef iterator const_iterator;

	// constructors
	trie(trie<T>* const = nullptr);
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

#endif
