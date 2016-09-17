#ifndef __VectorHash_H
#define __VectorHash_H

#include <vector>
#include <deque>

namespace std {

	template<class T>
	struct hash < vector<T> > {
		size_t operator()(const vector<T>& vec) const {
			size_t res = 0;
			for (size_t i = 0; i < vec.size(); ++i)
				res = res * 37 + internal_(vec[i]);
			return res;
		}
		std::hash<T> internal_;
	};

}

namespace std {

	template<class T>
	struct hash < std::deque<T> > {
		size_t operator()(const std::deque<T>& vec) const {
			size_t res = 0;
			for (size_t i = 0; i < vec.size(); ++i)
				res = res * 37 + internal_(vec[i]);
			return res;
		}
		std::hash<T> internal_;
	};

}

#endif

