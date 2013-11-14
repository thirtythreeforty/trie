#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <list>
#include <set>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <algorithm>
#include <cassert>
 
#include "trie.h"

using namespace std;
using namespace std::chrono;

template<typename T>
void readIntoContainer(T& t)
{
	std::ifstream inp("dict");
	while(inp.good()) {
		std::string line;
		inp >> line;
		t.insert(end(t), line);
	}
}

/** Returns duration of f(), by default in milliseconds */
template<typename Duration = milliseconds, typename UnaryPredicate>
Duration timeFunctionCall(UnaryPredicate f)
{
	auto t1 = chrono::high_resolution_clock::now();
	f();
	auto t2 = chrono::high_resolution_clock::now();
	return chrono::duration_cast<Duration>(t2 - t1);
}

template<typename T>
inline void outputFormat(const T& thing, ostream& stream = cout)
{
	stream << left << setw(20) << thing << flush;
}
void outputFormatHeader(string header)
{
	cout << header << '\n';
	for(const auto& i: {"iterations", "std::list", "std::set", "std::unordered_set", "std::vector", "gh403::trie"})
		outputFormat(i);
	cout << endl;
}

int main()
{
	const int largest = 1000000;
	vector<string> source;

	readIntoContainer(source);

	list<string>          lsrc(begin(source), begin(source) + largest);
	set<string>           ssrc(begin(source), begin(source) + largest);
	unordered_set<string> usrc(begin(source), begin(source) + largest);
	vector<string>        vsrc(begin(source), begin(source) + largest);
	trie<string>          tsrc(begin(source), begin(source) + largest);

	outputFormatHeader("INSERTION");
	for(int i = 0; i <= largest; i += 20000) {
		list<string>          *l = new list<string>;
		set<string>           *s = new set<string>;
		unordered_set<string> *u = new unordered_set<string>;
		vector<string>        *v = new vector<string>;
		trie<string>          *t = new trie<string>;

		vector<string> source_cpy(source.cbegin(), source.cbegin() + i);
		random_shuffle(begin(source_cpy), end(source_cpy));

		outputFormat(i);
		outputFormat(timeFunctionCall( [&]{ l->insert(l->begin(), source_cpy.begin(), source_cpy.end()); } ).count() );
		outputFormat(timeFunctionCall( [&]{ s->insert(source_cpy.begin(), source_cpy.end()); } ).count() );
		outputFormat(timeFunctionCall( [&]{ u->insert(source_cpy.begin(), source_cpy.end()); } ).count() );
		outputFormat(timeFunctionCall( [&]{ v->insert(v->begin(), source_cpy.begin(), source_cpy.end()); } ).count() );
		outputFormat(timeFunctionCall( [&]{ t->insert(source_cpy.begin(), source_cpy.end()); } ).count() );
		cout << endl;

		delete l;
		delete s;
		delete u;
		delete v;
		delete t;
	}
	cout << endl;

	outputFormatHeader("FIND (PRESENT KEY)");
	for(int i = 0; i <= largest; i += 20000) {
		//list<string>
		set<string>           *s = new set<string>(ssrc);
		unordered_set<string> *u = new unordered_set<string>(usrc);
		//vector<string>
		trie<string>          *t = new trie<string>(tsrc);

		vector<string> source_cpy(source.cbegin(), source.cbegin() + i);
		random_shuffle(begin(source_cpy), end(source_cpy));

		outputFormat(i);
		outputFormat("");
		outputFormat(timeFunctionCall( [&]{ for(auto n = source_cpy.cbegin(); n < source_cpy.cend(); ++n) s->find(*n); } ).count() );
		outputFormat(timeFunctionCall( [&]{ for(auto n = source_cpy.cbegin(); n < source_cpy.cend(); ++n) u->find(*n); } ).count() );
		outputFormat("");
        outputFormat(timeFunctionCall( [&]{ for(auto n = source_cpy.cbegin(); n < source_cpy.cend(); ++n) t->find(*n); } ).count() );
		cout << endl;

		delete s;
		delete u;
		delete t;
	}
	cout << endl;

	outputFormatHeader("FIND (RANDOM KEY)");
	for(int i = 0; i <= largest; i += 20000) {
		//list<string>
		set<string>           *s = new set<string>(ssrc);
		unordered_set<string> *u = new unordered_set<string>(usrc);
		//vector<string>
		trie<string>          *t = new trie<string>(tsrc);

		vector<string> source_cpy(source.cbegin(), source.cend());
		random_shuffle(begin(source_cpy), end(source_cpy));

		outputFormat(i);
		outputFormat("");
		outputFormat(timeFunctionCall( [&]{ for(auto n = source_cpy.cbegin(); n < source_cpy.cend(); ++n) s->find(*n); } ).count() );
		outputFormat(timeFunctionCall( [&]{ for(auto n = source_cpy.cbegin(); n < source_cpy.cend(); ++n) u->find(*n); } ).count() );
		outputFormat("");
        outputFormat(timeFunctionCall( [&]{ for(auto n = source_cpy.cbegin(); n < source_cpy.cend(); ++n) t->find(*n); } ).count() );
		cout << endl;

		delete s;
		delete u;
		delete t;
	}
	cout << endl;

	outputFormatHeader("DELETION (PRESENT KEY)");
	for(int i = 0; i <= largest; i += 20000) {
		//list<string>
		set<string>           *s = new set<string>(ssrc);
		unordered_set<string> *u = new unordered_set<string>(usrc);
		//vector<string>
		trie<string>          *t = new trie<string>(tsrc);

		vector<string> source_cpy(source.cbegin(), source.cbegin() + largest);
		random_shuffle(begin(source_cpy), end(source_cpy));

		outputFormat(i);
		outputFormat("");
		outputFormat(timeFunctionCall( [&]{ for(auto n = source_cpy.cbegin(); n < source_cpy.cbegin() + i; ++n) s->find(*n); } ).count() );
		outputFormat(timeFunctionCall( [&]{ for(auto n = source_cpy.cbegin(); n < source_cpy.cbegin() + i; ++n) u->find(*n); } ).count() );
		outputFormat("");
        outputFormat(timeFunctionCall( [&]{ for(auto n = source_cpy.cbegin(); n < source_cpy.cbegin() + i; ++n) t->find(*n); } ).count() );
		cout << endl;

		delete s;
		delete u;
		delete t;
	}
	cout << endl;

	outputFormatHeader("DELETION (FRONT)");
	for(int i = 0; i <= largest; i += 20000) {
		list<string>          *l = new list<string>(lsrc);
		set<string>           *s = new set<string>(ssrc);
		unordered_set<string> *u = new unordered_set<string>(usrc);
		vector<string>        *v = new vector<string>(vsrc);
		trie<string>          *t = new trie<string>(tsrc);

		outputFormat(i);
		outputFormat(timeFunctionCall( [&]{ auto it = l->begin(); for(int n = 0; n < i; ++n) it = l->erase(it); } ).count() );
		outputFormat(timeFunctionCall( [&]{ auto it = s->begin(); for(int n = 0; n < i; ++n) it = s->erase(it); } ).count() );
		outputFormat(timeFunctionCall( [&]{ auto it = u->begin(); for(int n = 0; n < i; ++n) it = u->erase(it); } ).count() );
		// We'd like the benchmarks to finish before the end of the semester.
		if(i <= 20000) outputFormat(timeFunctionCall( [&]{ auto it = v->begin(); for(int n = 0; n < i; ++n) it = v->erase(it); } ).count() );
		else           outputFormat("");
		outputFormat(timeFunctionCall( [&]{ auto it = t->begin(); for(int n = 0; n < i; ++n) it = t->erase(it); } ).count() );

		cout << endl;

		delete l;
		delete s;
		delete u;
		delete v;
		delete t;
	}
	cout << endl;

	outputFormatHeader("DELETION (REAR)");
	for(int i = 0; i <= largest; i += 20000) {
		list<string>          *l = new list<string>(lsrc);
		set<string>           *s = new set<string>(ssrc);
		//unordered_set<string> // lacks iterator::operator--()
		vector<string>        *v = new vector<string>(vsrc);
		trie<string>          *t = new trie<string>(tsrc);

		outputFormat(i);
		outputFormat(timeFunctionCall( [&]{ auto it = l->end(); for(int n = 0; n < i; ++n) it = l->erase(--it); } ).count() );
		outputFormat(timeFunctionCall( [&]{ auto it = s->end(); for(int n = 0; n < i; ++n) it = s->erase(--it); } ).count() );
		outputFormat("");
		outputFormat(timeFunctionCall( [&]{ auto it = v->end(); for(int n = 0; n < i; ++n) it = v->erase(--it); } ).count() );
		outputFormat(timeFunctionCall( [&]{ auto it = t->end(); for(int n = 0; n < i; ++n) it = t->erase(--it); } ).count() );

		cout << endl;

		delete l;
		delete s;
		delete v;
		delete t;
	}
	cout << endl;

	return 0;
}
