#pragma once
#include <string>

#include <iostream>

#include <vector>
#include <deque>
#include <unordered_map>
#include <map>

//#include <functional>

using namespace std;

/*
template <template <class> class C, class T>
ostream& operator<<(ostream& os, const C<T>& ob) {
	for (auto i : ob) os << i << " ";
	return os;
}

template <template <class> class C, class T, class S>
ostream& operator<<(ostream& os,
	const unordered_map<C<T>, S>& ob) {
	for (auto i : ob) os << i.first << " " << i.second << endl;
	return os;
}
*/

template <class T>
ostream& operator<<(ostream& os, const deque<T>& ob) {
	for (auto i : ob) os << i << " ";
	return os;
}

template <class T, class S>
ostream& operator<<(ostream& os,
	const unordered_map<deque<T>, S>& ob) {
	for (auto i : ob) os << i.first << " " << i.second << endl;
	return os;
}

template <class T>
ostream& operator<<(ostream& os, const vector<T>& ob) {
	for (auto i : ob) os << i << " ";
	return os;
}

template <class T, class S>
ostream& operator<<(ostream& os,
	const unordered_map<vector<T>, S>& ob) {
	for (auto i : ob) os << i << endl;
	return os;
}

//template <class T>
//ostream& operator<<(ostream& os,
//	const reference_wrapper<T>& ob) {
//	return os << ob.get();
//}

template <class T1, class T2>
ostream& operator<<(ostream& os,
	const pair<T1, T2>& p) {
	return os << p.first << " " << p.second;
}

template <class T1, class T2>
ostream& operator<<(ostream& os,
	const map<T1, T2>& m) {
	for (auto& p : m) os << p;
	return os;
}
