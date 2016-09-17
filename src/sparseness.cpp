#include <string>

#include <iostream>
#include "ioutils.h" // cout <<

#include "fileRead.h"

#include <vector>
#include <deque>
#include <unordered_map>
#include "VectorHash.h"

#include <algorithm> // min

using namespace std;

int main(int argc, char **argv)
{
	// parameter input
	string filenames[] = { "p1_a", "p1_b"}; 
	for (auto& filename : filenames) filename = filename + ".txt";
	int n = 2;
	bool want_print_common = true;
	bool want_cin_get_at_return = true;
	if (argc == 5) {
		filenames[0] = argv[1];
		filenames[1] = argv[2];
		n = stoi(argv[3]);
		want_print_common = stoi(argv[4]) > 0;
		want_cin_get_at_return = false;
	}

	vector<string> tokens[2];
	for (int I : {0, 1}) read_tokens(filenames[I], tokens[I], false);

	unordered_map<deque<string>, int> train_ngrams_counts;	
	auto r = tokens[0].begin() + min(n, int(tokens[0].size()));
	deque<string> ngram(tokens[0].begin(), r);
	while (true) {
		if (train_ngrams_counts.count(ngram) == 0) 
			train_ngrams_counts[ngram] = 1; 
		else 
			train_ngrams_counts[ngram]++;
		if (r == tokens[0].end()) break;
		ngram.pop_front();
		ngram.push_back(*r);
		r++;
	} 

	vector<deque<string>> test_ngrams;
	r = tokens[1].begin() + min(n, int(tokens[1].size()));
	ngram.assign(tokens[1].begin(), r);
	while (true) {
		test_ngrams.push_back(ngram);
		if (r == tokens[1].end()) break;
		ngram.pop_front();
		ngram.push_back(*r);
		r++;
	}

	int sparseness_counter = 0;
	for (auto& ngram : test_ngrams)
		if (train_ngrams_counts.count(ngram) > 0) {
			sparseness_counter++;
			if (want_print_common) cout << ngram << endl;
		}

	double sparsness_percentage = 
		100.0 * (1.0 - double(sparseness_counter) / test_ngrams.size());
	cout << sparsness_percentage << endl;

	return want_cin_get_at_return ? cin.get() : 0;
}
