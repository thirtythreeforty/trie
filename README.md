`trie` is a templatized trie container library written in C++11.  It aims to be
compatible with STL syntax.  It possesses bidirectional iterators and most 
(meaningful) STL methods.  It can be store any iterable container, such as 
`std::string` or `std::vector<T>`.  This `trie` was written as a project for a 
college course, and is fairly well documented throughout.  It is believed to be 
reasonably bug-free.  The writeup is included in `report.pdf`.

The trie consumes about the same amount of memory as a `std::set` containing the same 
data.  Unfortunately, performance is generally slightly worse than that of a 
`std::set`, mostly because of the iterator implementation.  The next logical 
improvement is to implement a full PATRICIA trie algorithm; this would substantially 
increase algorithm complexity.

`trie` is released under the GNU LGPL.  See the file COPYING for more details.  Pull 
requests and bug reports are welcome!
