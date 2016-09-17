#include <string>

#include <iostream>
#include "ioutils.h"
#include <sstream>

#include "fileRead.h"
#include <sys/stat.h> // check if saved model file exists

#include <vector>
#include <deque>
#include <unordered_map>
#include "VectorHash.h"

#include <numeric> // accumulate (sum)
#include <algorithm> // min
#include <cfloat>

#include "smoothing.h"
#include "utils.h"

using namespace std;

bool want_saved_model = true;

int main(int argc, char **argv)
{
	// parameters
	string filenames[3];
	int n;
	int threshold;
	double delta;
	Smoothing smoothing;
	bool want_get_at_return;
	string model_filename;
	{
		filenames[0] = "hugeTrain.txt";
		filenames[0] = "tinyTrain.txt";
		filenames[1] = "textCheck.txt";
		filenames[1] = "tinyCheck.txt";
		filenames[2] = "dictionary.txt";
		n = 2;
		threshold = 3;
		delta = 0.01;
		smoothing = Additive;
		want_get_at_return = true;
		if (argc == 8) {
			for (int i : {0, 1, 2}) filenames[i] = argv[i + 1];
			n = stoi(argv[4]);
			threshold = stoi(argv[5]);
			delta = stod(argv[6]);
			smoothing = stoi(argv[7]) == 0 ? Good_Turing : Additive;
			if (smoothing == Good_Turing) {
				cerr << "not implemented\n";
				return 1;
			}
			want_get_at_return = false;
		}
		model_filename = filenames[0] + "." +
			(smoothing == Additive ? "add" : "gt") + ".model";
		for (int i : {0, 1, 2}) filenames[i] = filenames[i];
	}
	n++;

	vector<string> dictionary; read_tokens(filenames[2], dictionary, false);

	vector<deque<string>> sentences[2];
	for (int I : {0, 1}) {
		vector<string> tokens; read_tokens(filenames[I], tokens, true);
		for (auto i = tokens.begin(), j = i; i != tokens.end(); ++i)
			if (*i == EOS) {
				sentences[I].emplace_back(j, i); 
				j = i + 1;
			}
	}
	double N = 0; for (auto& sentence : sentences[0]) N += sentence.size();

	//cout << "+ igrams_p\n";
	vector<unordered_map<deque<string>, double>> igrams_p(n);
	double unseen_p = DBL_MAX; // nvm, jst ignore
	bool need_to_save_model = false;
	struct stat stat_buffer;
	bool model_exists = stat(model_filename.c_str(), &stat_buffer) == 0;
	if (want_saved_model && model_exists) {
		ifstream file(model_filename);
		//{ double N; file >> N; igrams_p[0][deque<string>()] = N; file.get(); }
		file >> unseen_p; file.get();
		while (file.good()) {
			string line; getline(file, line);
			istringstream line_stream(line);
			string token;
			deque<string> buffer;
			while (line_stream >> token)
				buffer.push_back(token);
			if (buffer.empty() || buffer.size() >= size_t(n + 1)) break;
			igrams_p[buffer.size() - 1][deque<string>(buffer.begin(), buffer.end() - 1)] = stod(buffer.back());
		}
		file.close();
	}
	else {
		// calculate counts
		for (auto& sentence : sentences[0])
			for (size_t i = 0; i < min(size_t(n), sentence.size()); ++i) {
				auto l = sentence.begin(), r = l + i;
				while (true) {
					deque<string> igram(l, r);
					if (igrams_p[i].count(igram) == 0)
						igrams_p[i][igram] = 1;
					else
						igrams_p[i][igram]++;
					if (r == sentence.end()) break;
					l++;  r++;
				}
			}
		//double N = (igrams_p[0].begin()->second -= sentences[0].size());
		double N = accumulate(igrams_p[1].begin(), igrams_p[1].end(), 0.0,
			[](double ps, auto& abc) { return ps + abc.second; });
		double B = pow(N, double(n - 1));
		//double b = pow(dictionary.size(), double(n -1));

		if (smoothing == Good_Turing) {
			// recalculate counts
			vector<map<int, int>> Nc(n);
			for (int i = 0; i < n; i++)
				for (auto& igram_c : igrams_p[i]) {
					int c = int(igram_c.second);
					if (Nc[i].count(c) == 0) Nc[i][c] = 1; else Nc[i][c]++;
				}
			// all frequencies are greater than zero for counts below threshold
			for (int i = 0; i < n; i++)
				for (int k = 1; k <= threshold; k++)
					if (Nc[i].count(k) == 0) {
						cerr << "r is too high" << endl;
						return 1;
					}
			for (int i = 0; i < n; i++)
				for (auto& abc : igrams_p[i]) {
					auto& c = abc.second; int c_int = int(c);
					if (c_int < threshold) c = double(c + 1.0) * double(Nc[i][c_int + 1]) / double(Nc[i][c_int]);
				}
			for (int i = 0; i < n; ++i)
				for_each(igrams_p[i].begin(), igrams_p[i].end(),
					[&](auto& abc) {
						igrams_p[i][abc.first] = (
							double(abc.second) /
							double(N)
							);
					}
				);
			// renormalize, take log
			for (int i = 0; i < n; ++i) {
				double y = (1.0 - 1.0 / (N * B)) /
					accumulate(igrams_p[i].begin(), igrams_p[i].end(), 0.0,
						[](double ps, auto& abp) { return ps + abp.second; });
				for_each(igrams_p[i].begin(), igrams_p[i].end(),
					[&y](auto& a) { a.second = log(a.second * y); });
			}
			unseen_p = log(double(Nc[n - 1][1])) - log(N * B);
		}
		else { // smoothing == Additive
			double denominator = N + delta * B;
			for (int i = 0; i < n; ++i)
				for (auto& igram_p : igrams_p[i])
					igram_p.second = log(igram_p.second + delta) - log(denominator);
			unseen_p = log(delta) - log(denominator);
		}

		sentences[0].clear();
	}
	if (!model_exists) {
		ofstream file(model_filename);
		file << " " << unseen_p << endl;
		for (int i = 1; i < n; ++i) {
			for (auto& igram_p : igrams_p[i])
				file << igram_p << endl;
		}
		file.close();
	}
//	int N = 0; for (auto& sentence : sentences) N += sentence.size();
	
	unordered_map<string, vector<int>> indices_of_closest; // by levenshtein distance
	for (auto& sentence : sentences[1]) {
		//cout << "> " << sentence << endl;
		for (string& token : sentence) {
			if (indices_of_closest.count(token) == 0) {
				vector<int> buffer;
				for (size_t i = 0; i < dictionary.size(); ++i)
					// FIXME (optimization): no need to calc full distance
					if (abs(int(dictionary[i].size() - token.size())) <= 1 &&
						uiLevenshteinDistance(token, dictionary[i]) <= 1)
						buffer.push_back(i);
				indices_of_closest[token] = buffer;
			}
		}
		double p_max = -DBL_MAX;
		int I = -1, J = I;
		vector<string> changed(sentence.begin(), sentence.end());
		for (size_t i = 0; i < sentence.size(); ++i) {
			for (int closest_i : indices_of_closest[sentence[i]]) {
				changed[i] = dictionary[closest_i];
				deque<string> igram(1, changed.front());
				double p = 0;
				for (size_t k = 1; k < changed.size(); ++k) {
					if (igrams_p[igram.size()].count(igram) == 0) {
						p += unseen_p;
						//igrams_p[igram.size()][igram] = unseen_p;
					}
					else
						p += igrams_p[igram.size()][igram];
					if (igram.size() > 1) {
						auto subigram = deque<string>(igram.begin(), igram.end() - 1);
						if (false && igrams_p[igram.size() - 1].count(subigram) == 0)
							p -= unseen_p;
						else 
							p -= igrams_p[igram.size() - 1][subigram];
					}
					if (igram.size() == n - 1) igram.pop_front();
					igram.push_back(changed[k]);
				}
				if (p >= p_max) {
					p_max = p;
					I = i; J = closest_i;
					//cout << "I = " << I << ", J = " << J << endl;
				}
				//cout << changed << p << endl;
			}
			changed[i] = sentence[i];
		}
		if (I != -1)
			changed[I] = dictionary[J];
		cout /*<< "< "*/ << changed << endl;
	}

	cout << '\a'; // beeeep! (wake up)
	return want_get_at_return ? cin.get() : 0;
}
