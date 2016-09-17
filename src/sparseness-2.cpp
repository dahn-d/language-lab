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
	string filenames[] { "p2_train", "p2_test" };
	for (auto& filename : filenames) filename = path + filename + ".txt";
	int n = 2;
	bool want_cin_get_at_return = true;
	bool want_print = false;
	if (argc >= 4) {
		for (int I : {0, 1}) filenames[I] = argv[I+1];
		n = stoi(argv[3]);
		want_cin_get_at_return = false;
	}
	if (argc == 5) {
		want_print = true;
	}
	
	vector<deque<string>> sentences[2];
	{
		vector<string> tokens[2];
		for (int I : {0, 1}) read_tokens(filenames[I], tokens[I], true);
		for (int I : { 0, 1 }) {
			deque<string> buffer;
			for (auto& i : tokens[I]) {
				buffer.push_back(i);
				if (i == EOS) {
					sentences[I].push_back(buffer);
					buffer.clear();
				}
			}
		}
	}
	

	unordered_map<deque<string>, int> ngrams;	
	for (auto sentence : sentences[0]) {
		while (sentence.size() > 0) {
			for (int k = 1; k <= min(n, int(sentence.size())); ++k) {
				deque<string> sub_ngram(sentence.begin(), sentence.begin() + k);
				if (ngrams.count(sub_ngram) == 0)
					ngrams[sub_ngram] = 1;
				else
					ngrams[sub_ngram]++;
			}
			sentence.pop_front();
		}
	}

	int zero_count = 0;
	for (auto sentence : sentences[1]) {
		bool is_zero = false;
		for (int k = 1; k <= min(n, int(sentence.size())); ++k) {
			deque<string> kgram(sentence.begin(), sentence.begin() + k);
			if (ngrams.count(kgram) == 0) {
				zero_count++;
				is_zero = true;
				break;
			}
		}
		if (!is_zero && want_print) cout << sentence << endl;
		sentence.pop_front();
	}

	double zero_percentage = 100.0 * (double(zero_count) / double(sentences[1].size()));
	cout << zero_percentage << endl;

	return want_cin_get_at_return ? cin.get() : 0;
}