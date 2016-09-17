#include <string>

#include <iostream>
#include "ioutils.h"

#include "fileRead.h"

#include <vector>
#include <deque>
#include <unordered_map>
#include "VectorHash.h"

#include <numeric> // accumulate (sum)
#include <algorithm> // min
#include <cfloat>

#include "smoothing.h"

using namespace std;

#define VOCABULARY_SIZE_MULTIPLIER 2

int main(int argc, char **argv)
{
	// parameter input
	string filenames[] {
		"p4_textModel",
		"p4_sentences"
	};
	for (auto& filename : filenames) filename = filename + ".txt";
	int n = 2;
	//Smoothing smoothing = Additive;
	Smoothing smoothing = Good_Turing;
	double parameter = 2; // model parameter
	bool want_get_at_return = true;
	if (argc == 6) {
		filenames[0] = argv[1];
		filenames[1] = argv[2];
		n = stoi(argv[3]);
		parameter = stod(argv[4]);
		smoothing = stoi(argv[5]) == 0 ? Good_Turing : Additive;
		want_get_at_return = false;
	}

	vector<deque<string>> sentences[2];
	for (int I : {0, 1}) {
		vector<string> tokens; read_tokens(filenames[I], tokens, true);
		deque<string> buffer;
		for (auto& i : tokens)
			if (i == EOS) { sentences[I].push_back(buffer); buffer.clear(); }
			else buffer.push_back(i);
	}

	//for (auto ss : sentences[0]) cout << ss << endl; cout << endl;

	// first vector to access igrams by size (index+1)
	vector<unordered_map<deque<string>, double>> igrams_c;
	vector<vector<deque<string>>> igrams;
	igrams_c.resize(n);
	igrams.resize(n);
	for (auto sentence : sentences[0])
		while (!sentence.empty()) { // FIXME: can calculate in 1 pass
			for (int k = 0; k < min(n, int(sentence.size())); ++k) {
				deque<string> kgram(sentence.begin(), sentence.begin() + k + 1);
				if (igrams_c[k].count(kgram) == 0) {
					igrams_c[k][kgram] = 1;
					igrams[k].push_back(kgram);
				}
				else igrams_c[k][kgram]++;
			}
			sentence.pop_front();
		}

	//cout << igrams_c << endl;

	double N = accumulate(igrams_c[0].begin(), igrams_c[0].end(), 0.0,
		[&igrams_c](double ps, auto& unigram) { return ps + igrams_c[0][unigram.first]; });
	double B = pow(double(igrams_c[0].size() * VOCABULARY_SIZE_MULTIPLIER), double(n));

	vector<unordered_map<deque<string>, double>> igrams_p(n);
	vector<map<int, int>> Nc(n); // frequencies of frequencies (for GT)
	double unseen_p;

	switch (smoothing)
	{
		case Additive:
		{
			double denominator = double(N) + parameter * double(B);
			for (int i = 0; i < n; ++i) {
				for_each(igrams_c[i].begin(), igrams_c[i].end(),
					[&](auto& abc) {
					igrams_p[i][abc.first] = 
						log(double(abc.second) + parameter)
						- log(denominator);
					}
				);
			}
			unseen_p = log(parameter) - log(denominator);
			break;
		}
		case Good_Turing:
		{
			for (int i = 0; i < n; i++) {
				for (auto& igram_c : igrams_c[i]) {
					int c = int(igram_c.second);
					if (Nc[i].count(c) == 0) Nc[i][c] = 1; else Nc[i][c]++;
				}
			}
			for (int i = 0; i < n; i++) {
				for (int k = 1; k <= parameter; k++)
					if (Nc[i].count(k) == 0) {
						cerr << "r is too high" << endl;
						return 1;
					}
			}
			//for (auto& Ni : Nc) { for (auto& m : Ni) cout << m << endl; cout << endl; }
			for (int i = 0; i < n; i++) {
				for (auto& abc : igrams_c[i]) {
					auto& c = abc.second;
					if (c < parameter)
						c = double(c + 1) * Nc[i][int(c + 1)] / Nc[i][int(c)];
				}
			}
			//cout << igrams_c << endl;
			for (int i = 0; i < n; ++i) {
				for_each(igrams_c[i].begin(), igrams_c[i].end(),
					[&](auto& abc) {
					igrams_p[i][abc.first] = (
						double(abc.second) /
						//double(igrams_c[i - 1][deque<string>(abc.first.begin(), abc.first.end() - 1)])
						double(N)
						);
				}
				);
			}
			// renormalize
			for (int i = 0; i < n; ++i) {
				double y = (1.0 - 1.0 / (double(N) * double(B))) /
					accumulate(igrams_p[i].begin(), igrams_p[i].end(), 0.0,
						[](double ps, auto& abp) { return ps + abp.second; });
				for_each(igrams_p[i].begin(), igrams_p[i].end(),
					[&y](auto& a) { a.second = log(a.second * y); });
			}
			unseen_p = log(double(Nc[n - 1][1])) - log(N * B);
			break;
		}
	}

	//for (int i = 0; i < igrams_p.size(); ++i)
	//	for (auto& abp : igrams_p[i]) cout << abp << endl; cout << endl;
	
	for (auto& sentence : sentences[1]) {
		//cout << sentence << endl;
		deque<string> igram;
		double p = 0;
		//auto r = sentence.begin() + min(n, int(sentence.size()));
		//igram.assign(sentence.begin(), r);
		auto r = sentence.begin() + 1;
		igram.assign(sentence.begin(), r);
		while (true) {
			//if (igram.size() == n) 
			if (igrams_p[igram.size() - 1].count(igram) == 0) {
				p += unseen_p;
			}
			else {
				p += igrams_p[igram.size() - 1][igram];
			}
			if (igram.size() > 1) {
				auto subigram = deque<string>(igram.begin(), igram.end() - 1);
				if (igrams_p[igram.size() - 2].count(subigram) == 0)
					p -= unseen_p;
				else
					p -= igrams_p[igram.size() - 2][subigram];
			}
			//cout << igram << " " << p << endl;
			if (r == sentence.end()) break;
			if (igram.size() == n) igram.pop_front();
			igram.push_back(*r++);
		}
		cout << (isinf(p) ? -DBL_MAX : p) << endl;
	}

	return want_get_at_return ? cin.get() : 0;
}
