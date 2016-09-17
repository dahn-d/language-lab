#include <string>

#include <iostream>
#include "ioutils.h"

#include "fileRead.h"

#include <vector>
#include <deque>
#include <unordered_map>
#include "VectorHash.h"

//#include <algorithm> // unique, distance

#include <numeric> // accumulate (sum)

#include <cstdlib> // srand, rand
#include <ctime> // for srand(time(0))

#include <algorithm> // min

#include "utils.h"


using namespace std;

int main(int argc, char **argv)
{
	string filename = "p3.txt";
	int n = 2;
	bool want_get_at_return = true;
	if (argc == 3) {
		filename = argv[1];
		n = stoi(argv[2]);
		want_get_at_return = false;
	}

	vector<deque<string>> sentences;
	{
		vector<string> tokens; read_tokens(filename, tokens, true);
		deque<string> buffer;
		buffer.clear();
		for (auto& i : tokens) {
			buffer.push_back(i);
			if (i == EOS) {
				sentences.push_back(buffer);
				buffer.clear();
			}
		}
	}

	vector<unordered_map<deque<string>, int>> igrams_counts(n);
	for (auto sentence : sentences)
		while (!sentence.empty()) {
			for (int k = 1; k <= min(n, int(sentence.size())); ++k) {
				deque<string> kgram(sentence.begin(), sentence.begin() + k);
				if (igrams_counts[k - 1].count(kgram) == 0 && kgram[0] != EOS)
					igrams_counts[k - 1][kgram] = 1;
				else
					igrams_counts[k - 1][kgram]++;
			}
			sentence.pop_front();
		}

	int N = accumulate(igrams_counts[0].begin(), igrams_counts[0].end(), 0,
		[](int ps, auto& abc) { return ps + abc.second; });

	srand((unsigned)time(0));

	deque<string> tokens;
	{
		vector<double> agrams_ml(igrams_counts[0].size());
		vector<deque<string>> agrams(agrams_ml.size());
		if (agrams.size() == 0) cerr << "rekt 0\n";

		// calculate probabilities from counts
		transform(igrams_counts[0].begin(), igrams_counts[0].end(), agrams_ml.begin(),
			[&N](auto& abc) { return double(abc.second) / double(N); });
		transform(igrams_counts[0].begin(), igrams_counts[0].end(), agrams.begin(),
			[](auto& abc) { return abc.first; });
		// generate first n-1 tokens (initial context)
		// 0
		while (tokens.empty()) {
			auto& token = agrams[drawIndex(agrams_ml)].back();
			if (token != EOS) tokens.push_back(token);
		}
		// 1,...,n-1
		for (int i = 1; i < n && tokens.back() != EOS; ++i) {
			agrams_ml.clear();
			agrams.clear();
			for_each(igrams_counts[i].begin(), igrams_counts[i].end(),
				[&](auto& abc) {
				if (equal(tokens.begin(), tokens.end(), abc.first.begin())) {
					agrams.push_back(abc.first);
					agrams_ml.push_back(double(abc.second) / double(igrams_counts[i - 1][tokens]));
				}
			});
			if (agrams.empty()) { cerr << "err: exhausted \n"; break; }
			int index = drawIndex(agrams_ml);
			tokens.push_back(agrams[index].back());
		}

		// generate the rest of n-grams
		while (tokens.back() != EOS) {
			agrams_ml.clear();
			agrams.clear();
			for_each(igrams_counts[n - 1].begin(), igrams_counts[n - 1].end(),
				[&](auto& abc) {
				deque<string> a(tokens.end() - n + 1, tokens.end());
				if (equal(a.begin(), a.end(), abc.first.begin())) {
					agrams.push_back(abc.first);
					agrams_ml.push_back(double(abc.second) / (n == 1 ? double(N) : double(igrams_counts[n - 2][a])));
				}
			});
			if (agrams.empty()) { cerr << "err: exhausted \n"; break; }
			int index = drawIndex(agrams_ml);
			tokens.push_back(agrams[index].back());
		}
	}
	
	tokens.pop_back(); // remove EOS
	//tokens[0][0] -= 32; // capitalize
	cout << tokens;
	//cout << "\b." << endl; // fullstop
	
	return want_get_at_return ? cin.get() : 0;
}
