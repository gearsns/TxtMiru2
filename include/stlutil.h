#ifndef __STLUTIL_H__
#define __STLUTIL_H__
#include "stltchar.h"
#include <regex>
#include <windows.h>
namespace std {
	// for_each の引数を簡略化するテンプレート関数
	template <typename T_container, typename T_function>
	T_function for_each(T_container& rcontainer, T_function function) {
		return for_each(rcontainer.begin(), rcontainer.end(), function);
	}
	__declspec(noinline) tstring& replace(tstring& str, LPCTSTR src, LPCTSTR dst);
	template <class _Ty>
	class simple_array {
	private:
		_Ty *p = nullptr;
		int inum = 0;
	public:
		simple_array(){}
		~simple_array(){ if(p){ delete [] p; } }
		inline int size() const { return inum; }
		void resize(int n){
			if(p){ delete [] p; }
			if(n == 0){
				p = nullptr;
			} else {
				p = new _Ty[n];
			}
			if(p){
				inum = n;
			} else {
				inum = 0;
			}
		}
		bool empty() const { return p == nullptr; }
		_Ty &operator[](int i){ return p[i]; }
		_Ty *data() const{ return p; }
	};
	void replaceCRLF(std::tstring &str, LPCTSTR repstr);
	template<class BidirIt, class Traits, class CharT, class UnaryFunction>
	std::basic_string<CharT> regex_replace(BidirIt first, BidirIt last,
										   const std::basic_regex<CharT, Traits>& re, UnaryFunction f)
	{
		std::basic_string<CharT> s;

		typename std::match_results<BidirIt>::difference_type positionOfLastMatch = 0;
		auto endOfLastMatch = first;

		auto callback = [&](const std::match_results<BidirIt>& match)
		{
			auto positionOfThisMatch = match.position(0);
			auto diff = positionOfThisMatch - positionOfLastMatch;

			auto startOfThisMatch = endOfLastMatch;
			std::advance(startOfThisMatch, diff);

			s.append(endOfLastMatch, startOfThisMatch);
			s.append(f(match));

			auto lengthOfMatch = match.length(0);

			positionOfLastMatch = positionOfThisMatch + lengthOfMatch;

			endOfLastMatch = startOfThisMatch;
			std::advance(endOfLastMatch, lengthOfMatch);
		};

		std::regex_iterator<std::basic_string<CharT>::const_iterator> begin(first, last, re), end;
		std::for_each(begin, end, callback);

		s.append(endOfLastMatch, last);

		return s;
	}

	template<class Traits, class CharT, class UnaryFunction>
	std::basic_string<CharT> regex_replace(const std::basic_string<CharT> & s,
										   const std::basic_regex<CharT, Traits>& re, UnaryFunction f)
	{
		return regex_replace(s.cbegin(), s.cend(), re, f);
	}
	//
	template<class T>
	class const_reverse_wrapper {
		const T& container;
	public:
		const_reverse_wrapper(const T& container) : container(container){ }
		auto begin() const -> decltype(container.rbegin()) { return container.rbegin(); }
		auto edn() const -> decltype(container.rend()) { return container.rend(); }
	};

	template<class T>
	class reverse_wrapper {
		T& container;
	public:
		reverse_wrapper(T& container) : container(container){ }
		auto begin() -> decltype(container.rbegin()) { return container.rbegin(); }
		auto edn() -> decltype(container.rend()) { return container.rend(); }
	};

	template<class T>
	const_reverse_wrapper<T> reverse(const T& container) {
		return const_reverse_wrapper<T>(container);
	}

	template<class T>
	reverse_wrapper<T> reverse(T& container) {
		return reverse_wrapper<T>(container);
	}
	bool utf82w(LPCSTR c, std::tstring &w);
	bool w2utf8(LPCTSTR w, std::string &c, UINT iCodePage = CP_UTF8);
};

#endif // __STLUTIL_H__
