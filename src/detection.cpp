#include <string>

#include <iostream>
#include "ioutils.h"

#include "fileRead.h"
#include <sys/stat.h> // check if saved model file exists

#include <vector>
#include <deque>
#include <unordered_map>
#include "VectorHash.h"

#include <numeric> // accumulate (sum)
#include <algorithm> // min

#include "smoothing.h"

using namespace std;

int V = 256; // vocabulary size
vector<string> languages {
	//*
	"danish",
	"english",
	"french",
	"italian",
	"latin",
	"sweedish"/*/
	"asdasd",
	"ohoho"//*/
};
bool want_latin_only = false;
bool want_counts_saved = true;

int main(int argc, char **argv)
{
	string path = argv[1];

	// parameters
	size_t n;
	double delta;
	int sentence_length;
	bool want_get_at_return;
	{
		n = 1;
		delta = 0.05;
		sentence_length = 50;
		want_get_at_return = false;
		if (argc == 5) {
			n = stoi(argv[2]);
			delta = stod(argv[3]);
			sentence_length = stoi(argv[4]);
			want_get_at_return = false;
		}
	}
	n++; // hax

	int Vp = int(pow(V, n - 1));
	
	map<string, vector<unordered_map<deque<char>, int>>> igrams_c;
	map<string, int> N;
	{
		map<string, vector<char>> tokens;
		for (string language : languages) {
			read_tokens(path + language + "1.txt", tokens[language], want_latin_only);
			N[language] = tokens[language].size();
			igrams_c[language].resize(n);
			{
				struct stat stat_buffer;
				if (want_counts_saved && stat((path + language + ".counts").c_str(), &stat_buffer) == 0) {
					ifstream file(path + language + ".counts");
					{ int N; file >> N; igrams_c[language][0][deque<char>()] = N; }
					while (file.good()) {
						char c;
						file >> c;
						deque<char> buffer;
						while (c != ' ') {
							buffer.push_back(c);
							file >> noskipws >> c >> c; // skip 1 (whitespace)
						}
						if (buffer.empty() || buffer.size() >= n) break;
						int C;
						file >> C;
						igrams_c[language][buffer.size()][buffer] = C;
						file.get();
					}
					file.close();
				}
				else
				{
					for (size_t i = 0; i < n; ++i) {
						auto l = tokens[language].begin();
						auto r = l + i;
						while (true) {
							deque<char> igram(l, r);
							if (igrams_c[language][i].count(igram) == 0)
								igrams_c[language][i][igram] = 1;
							else
								igrams_c[language][i][igram]++;
							if (r == tokens[language].end()) break;
							l++;
							r++;
						}
					}
					igrams_c[language][0][deque<char>()]--;
					if (want_counts_saved) {
						ofstream file(path + language + ".counts");
						for (size_t i = 0; i < n; ++i) {
							for (auto& igram_c : igrams_c[language][i])
								file << igram_c << endl;
						}
						file.close();
					}
				}
			}
		}
	}

	//for (auto& language : languages)
	//for (int i = 0; i < n; ++i) 
	//for (auto& igram_c : igrams_c[language][i])
	//	cout << igram_c << endl;
	
	map<string, vector<unordered_map<deque<char>, double>>> igrams_p;

	map<string, double> denominator;
	map<string, double> unseen_p;
	for (auto& language : languages) {
		denominator[language] = double(N[language]) + delta * double(Vp);
		unseen_p[language] = log(delta / denominator[language]);
		igrams_p[language].resize(n);
		for (size_t i = 1; i < n; ++i) {
			for_each(igrams_c[language][i].begin(), igrams_c[language][i].end(),
				[&](auto& igram_c) {
					igrams_p[language][i][igram_c.first] = log(
						double(igram_c.second + delta) / denominator[language]
						);
				}
			);
		}
	}
	
	map<string, vector<vector<char>>> test_sentences; // testing data
	{
		map<string, vector<char>> test;
		for (auto& language : languages) {
			read_tokens(path + language + "2.txt", test[language], want_latin_only);
			vector<char> sentence; sentence.reserve(sentence_length);
			for (size_t i = 0; i < test[language].size(); ++i) {
				sentence.push_back(test[language][i]);
				if ((i+1) % sentence_length == 0) {
					test_sentences[language].push_back(sentence);
					sentence.clear();
				}
			}
		}
	}

	vector<vector<int>> confusion(languages.size());
	int sentences_total = 0;
	int correct_total = 0;
	for (auto& row : confusion) row.resize(languages.size());
	for (size_t i = 0; i < languages.size(); ++i) {
		for (auto& sentence : test_sentences[languages[i]]) {
			vector<double> p(languages.size(), 0.0);
			for (size_t j = 0; j < languages.size(); ++j) {
				auto& model = igrams_p[languages[j]];
				deque<char> igram(1, sentence.front());
				p[j] = model[1][igram];
				for (size_t k = 1; k < sentence.size(); ++k) {
					if (n - 1 == igram.size()) igram.pop_front();
					igram.push_back(sentence[k]);
					if (model[igram.size()].count(igram) == 0)
						p[j] += unseen_p[languages[j]];
					else
						p[j] += model[igram.size()][igram];
							//- model[igram.size()-1][deque<char>(igram.begin(), igram.end()-1)];
					p[j] -= model[igram.size() - 1][deque<char>(igram.begin(), igram.end() - 1)];
				}
			}
			int j = distance(p.begin(), max_element(p.begin(), p.end()));
			//cout << p << endl;
			confusion[i][j]++;
			if (i == j) correct_total++;
			sentences_total++;
		}
	}

	cout << /*sentences_total << " " << correct_total << " " <<*/ 
		100 * (1.0 - double(correct_total) / sentences_total) << endl;
	for (auto& row : confusion) {
		cout << row << endl;
	}

	return want_get_at_return ? cin.get() : 0;
}
