template<typename T>
class trie<T>::iterator {
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
	iterator() =default;
	iterator(trie<T>* node) {
		parents.emplace(node, node->children.cbegin());
		at_end = (parents.top().node_map_it == parents.top().node->children.cend());
		at_leaf = parents.top().node->is_leaf;
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
		if(at_leaf)
			at_leaf = false;
		else
			while(++parents.top().node_map_it == parents.top().node->children.cend())
				walk_up();
		fall_down();
	}
	void operator--() {
		if(at_leaf)
			walk_up();
		while(parents.top().node_map_it-- == parents.top().node->children.cbegin()) {
			at_leaf = parents.top().node->is_leaf;
			if(at_leaf)
				// No need to fall down.
				return;
			walk_up();
		}
		fall_down(fall_to::right);
	}

	bool operator==(const typename trie<T>::iterator& other) const {
		return parents.top() == other.parents.top() && at_end == other.at_end;
	}
	bool operator!=(const typename trie<T>::iterator& other) const { return !operator==(other); }
private:
	iterator(const std::stack<state>& parents, const T& built, bool at_end) :
		parents{parents}, built{built}, at_end{at_end} {}
	void inline fall_down(const enum fall_to fall = fall_to::left) {
		// TODO: This function could possibly be made smaller.
		trie<T>* child;
		if(at_leaf)
			return;
		while((child = parents.top().node_map_it->second.get()) != nullptr) {
			built.push_back(parents.top().node_map_it->first);
			if(fall == fall_to::left) {
				parents.emplace(child, child->children.cbegin());
				at_leaf = child->is_leaf;
				if(at_leaf)
					return;
			}
			else // fall_to::right
				parents.emplace(child, --child->children.cend());
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
	bool at_end;
	bool at_leaf;
};
