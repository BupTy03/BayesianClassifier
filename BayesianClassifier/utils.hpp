#pragma once

#include <map>
#include <set>
#include <string>
#include <cassert>
#include <tuple>
#include <utility>
#include <algorithm>
#include <functional>
#include <type_traits>


struct IsAlpha
{
	using argument_type = char;
	using result_type = bool;

	bool operator()(char ch) const
	{
		return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
			(ch >= 'À' && ch <= 'ß') || (ch >= 'à' && ch <= 'ÿ');
	}
};

struct ToLower
{
	using argument_type = char;
	using result_type = char;

	char operator()(char ch) const
	{
		if (ch >= 'À' && ch <= 'ß')
			return 'à' + ch - 'À';

		if (ch >= 'A' && ch <= 'Z')
			return 'a' + ch - 'A';

		return ch;
	}
};

struct WordsCounter
{
	explicit WordsCounter(std::map<std::string, std::size_t>& storage) : pCounts_{ &storage } { }

	void operator()(std::string::const_iterator first, std::string::const_iterator last)
	{
		if (first == last) return;

		buffer_.resize(std::distance(first, last));
		std::transform(first, last, buffer_.begin(), ToLower());

		auto it = pCounts_->find(buffer_);
		if (it == pCounts_->end())
		{
			std::string newWord;
			buffer_.swap(newWord);
			pCounts_->emplace(std::move(newWord), 1);
		}
		else
			it->second++;
	}

private:
	std::string buffer_;
	std::map<std::string, std::size_t>* pCounts_;
};

struct WordsSplitter
{
	explicit WordsSplitter(std::set<std::string>& storage) : pWords_{ &storage } {}
	void operator()(std::string::const_iterator first, std::string::const_iterator last)
	{ 
		std::string word(std::distance(first, last), '\0');
		std::transform(first, last, word.begin(), ToLower());
		pWords_->emplace(std::move(word)); 
	}

private:
	std::set<std::string>* pWords_;
};

template<typename Func1, typename Func2>
struct FunctorsAgregator
{
	explicit FunctorsAgregator(Func1&& f1, Func2&& f2)
		: functors(std::forward<Func1>(f1), std::forward<Func2>(f2)) {}

	template<typename... Args>
	void operator()(Args&&... args)
	{
		functors.first(std::forward<Args>(args)...);
		functors.second(std::forward<Args>(args)...);
	}

private:
	std::pair<Func1, Func2> functors;
};

template<typename... Funcs>
FunctorsAgregator<Funcs...> combine(Funcs&&... f)
{
	return FunctorsAgregator<Funcs...>(std::forward<Funcs>(f)...);
}


template<typename Container, typename KeyT, typename ValueT, typename = std::enable_if_t<std::is_fundamental_v<ValueT>>>
ValueT value_or_default(const Container& cont, const KeyT& key, ValueT value)
{
	auto it = cont.find(key);
	return (it == cont.end()) ? value : it->second;
}

template<typename InputIter, typename Func>
void split_words(InputIter first, InputIter last, Func&& f)
{
	while (true)
	{
		auto wordBegin = std::find_if(first, last, IsAlpha());
		auto wordEnd = std::find_if(wordBegin, last, std::not_fn(IsAlpha()));

		if (wordBegin == wordEnd)
			break;

		f(wordBegin, wordEnd);
		first = wordEnd;
	}
}

template<typename Container, typename Func>
void split_words(const Container& cont, Func&& f)
{
	split_words(std::begin(cont), std::end(cont), f);
}

std::set<std::string> to_words_set(const std::string& phrase)
{
	std::set<std::string> words;
	split_words(phrase.cbegin(), phrase.cend(), WordsSplitter(words));
	return words;
}
