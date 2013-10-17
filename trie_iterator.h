#ifndef TRIE_ITERATOR_H
#define TRIE_ITERATOR_H

#include <iterator>

#include "trie.h"

template<typename T>
class trie<T>::iterator : public std::iterator<std::bidirectional_iterator_tag, T> {
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
	enum class fall_to {left, right};
public:
	typedef T value_type;
	iterator() =default;
	iterator(const trie<T>* node) {
		parents.emplace(node, node->children.cbegin());
		at_end = (parents.top().node_map_it == parents.top().node->children.cend());
		at_leaf = parents.top().node->is_leaf;
		fall_down();
	}
	~iterator() =default;
	iterator(const typename trie<T>::iterator& other) =default;
	iterator(const typename trie<T>::iterator&& other) :
		parents{std::move(other.parents)},
		built{std::move(other.built)},
		at_end{other.at_end},
		at_leaf{other.at_leaf}
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

	trie<T>::iterator& operator++() {
		remove_state_and_advance();

		// Handle consequences of advance
		fall_down();
		return *this;
	}
	trie<T>::iterator& operator--() {
		while(!can_go_back()) {
			if(!at_leaf)
				built.pop_back();
			if(parents.size() == 1) {
				at_end = true;
				return *this;
			}
			else {
				parents.pop();
				at_leaf = false;
			}
		}
		remove_state_and_regress();
		while(!at_valid_leaf())
			step_down(false);
		step_down(false);
		return *this;
	}

	bool operator==(const typename trie<T>::iterator& other) const {
		return parents.top() == other.parents.top() && at_end == other.at_end;
	}
	bool operator!=(const typename trie<T>::iterator& other) const { return !operator==(other); }
private:
	iterator(const std::stack<state>& parents, const T& built, bool at_end, bool at_leaf) :
		parents{parents}, built{built}, at_end{at_end}, at_leaf{at_leaf} {}
	void fall_down(const enum fall_to fall = fall_to::left) {
		while(!at_valid_leaf()) {
			if(parents.top().node_map_it == parents.top().node->children.cend()) {
				if(parents.size() == 1) {
					at_end = true;
					return;
				}
				else {
					parents.pop();
					remove_state_and_advance();
				}
			}
			else
				step_down();
		}
		step_down();
	}
	void inline walk_up() {
		built.pop_back();
		parents.pop();
	}
	void inline remove_state_and_advance() {
		// Remove old state and advance
		if(!at_leaf) {
			built.pop_back();
			++parents.top().node_map_it;
		}
		else
			at_leaf = false;
	}
	bool inline can_go_back() {
		return (!at_leaf && parents.top().node->is_leaf) ||
		       (parents.top().node_map_it != parents.top().node->children.cbegin()) ||
		       (at_end && !parents.top().node->children.empty());
	}
	void inline remove_state_and_regress() {
		// This function assumes we can_go_back().
		if(at_end)
			at_end = false;
		else
			built.pop_back();
		if(parents.top().node_map_it == parents.top().node->children.cbegin())
			at_leaf = true;
		else
			--parents.top().node_map_it;
	}
	bool inline at_valid_leaf() {
		return parents.top().node_map_it != parents.top().node->children.cend() &&
		       (at_leaf ||
		        (parents.top().node_map_it->second.get() == nullptr));
	}
	void step_down(bool forward = true) {
		if(!at_leaf) {
			built.push_back(parents.top().node_map_it->first);
			if(parents.top().node_map_it->second.get() != nullptr) {
				parents.emplace( parents.top().node_map_it->second.get(),
				                 forward?
				                  parents.top().node_map_it->second.get()->children.cbegin() :
				                  --parents.top().node_map_it->second.get()->children.cend() );
				if(forward)
					at_leaf = parents.top().node->is_leaf;
			}
		}
	}

	std::stack<state> parents;
	// TODO: we could switch the use of push_back and pop_back for insert and erase
	// using an end iterator, to gain some additional compatibility.
	T built;
	bool at_end;
	bool at_leaf;
};

#endif
