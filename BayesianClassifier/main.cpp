#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <utility>
#include <numeric>
#include <functional>
#include <cassert>
#include <algorithm>
#include <cmath>


struct IsAlpha 
{
	using argument_type = char;
	using result_type = bool;

	bool operator()(char ch) const 
	{
		return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
			(ch >= 'А' && ch <= 'Я') || (ch >= 'а' && ch <= 'я');
	}
};

struct ToLower 
{
	using argument_type = char;
	using result_type = char;

	char operator()(char ch) const 
	{
		if (ch >= 'А' && ch <= 'Я')
			return 'a' + ch - 'A';

		if (ch >= 'A' && ch <= 'Z')
			return 'a' + ch - 'A';

		return ch;
	}
};

struct WordsCounter
{
	explicit WordsCounter(std::map<std::string, std::size_t>& storage) : pCounts_{&storage} { }

	void operator()(std::string::const_iterator first, std::string::const_iterator last)
	{
		assert(first != last);

		std::string word(std::distance(first, last), '\0');
		std::transform(first, last, word.begin(), ToLower());

		auto it = pCounts_->find(word);
		if (it == pCounts_->end())
			pCounts_->emplace(std::move(word), 1);
		else
			it->second++;
	}

private:
	std::map<std::string, std::size_t>* pCounts_;
};

struct WordsSplitter
{
	explicit WordsSplitter(std::set<std::string>& storage) : pWords_{&storage} {}

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
		std::get<Func1>(functors)(std::forward<Args>(args)...);
		std::get<Func2>(functors)(std::forward<Args>(args)...);
	}

	std::tuple<Func1, Func2> functors;
};

template<typename Func1, typename Func2>
FunctorsAgregator<Func1, Func2> add(Func1&& f1, Func2&& f2)
{
	return FunctorsAgregator<Func1, Func2>(std::forward<Func1>(f1), std::forward<Func2>(f2));
}



template<typename InputIter, typename Func>
void split_words(InputIter first, InputIter last, Func&& f)
{
	while (true)
	{
		auto wordBegin = std::find_if(first, last, IsAlpha());
		auto wordEnd = std::find_if(wordBegin, last, std::not1(IsAlpha()));

		if (wordBegin == wordEnd)
			break;

		f(wordBegin, wordEnd);
		first = wordEnd;
	}
}


std::size_t bayesian_classify(const std::vector<std::string>& classes,
							  const std::vector<std::string>& documents,
							  const std::vector<std::size_t>& mapping,
							  const std::string& newSample)
{
	assert(documents.size() == mapping.size());

	std::vector<std::map<std::string, std::size_t>> countWordsForClass(classes.size());
	std::set<std::string> allWordsInDocuments;
	for (std::size_t documentID = 0; documentID < documents.size(); ++documentID)
	{
		const auto& document = documents.at(documentID);
		const auto classID = mapping.at(documentID);
		auto& classDict = countWordsForClass.at(classID);

		split_words(document.cbegin(), document.cend(), add(WordsCounter(classDict), WordsSplitter(allWordsInDocuments)));
	}

	const auto countWordsInSample = allWordsInDocuments.size();

	std::set<std::string> newSampleWords;
	split_words(newSample.cbegin(), newSample.cend(), WordsSplitter(newSampleWords));

	constexpr double ALPHA = 1.0;
	std::pair<double, std::size_t> result(-std::numeric_limits<double>::infinity(), 0);
	for (std::size_t classID = 0; classID < classes.size(); ++classID)
	{
		const auto countDocumentsOfClass = static_cast<double>(std::count(mapping.cbegin(), mapping.cend(), classID));
		std::cout << '[' << classes.at(classID) << "]\n";
		std::cout << "log10(" << countDocumentsOfClass << " / " << mapping.size() << ")\n";

		double estimation = std::log10(countDocumentsOfClass / static_cast<double>(mapping.size()));
		for (const auto& word : newSampleWords)
		{
			const auto& dict = countWordsForClass.at(classID);
			const auto it = dict.find(word);
			const auto countOccurrencesOfWord = (it == dict.end()) ? 0 : it->second;

			std::cout << "log10((" << ALPHA << " + " << countOccurrencesOfWord << ") / (" << countWordsInSample << " + " << dict.size() << "))\n";
			estimation += std::log10((ALPHA + countOccurrencesOfWord) / static_cast<double>(countWordsInSample + dict.size()));
		}

		std::cout << "= " << estimation << '\n' << std::endl;

		if (estimation > result.first)
			result = { estimation, classID };
	}

	return result.second;
}


int main()
{
	setlocale(LC_ALL, "Russian");

	constexpr double ALPHA = 1.0;

	const std::vector<std::string> classes = { "Spam", "Ham" };
	const std::vector<std::string> documents = { "Предоставляю услуги бухгалтера", "Спешите купить iPhone", "Надо купить молоко" };
	const std::vector<std::size_t> mapping = { 0, 0, 1 };
	const std::string newSample = "надо купить услуги iPhone";

	std::cout << "Result: " << classes.at(bayesian_classify(classes, documents, mapping, newSample)) << std::endl;
	return 0;
}
