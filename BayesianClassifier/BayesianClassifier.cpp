#include "BayesianClassifier.hpp"
#include "utils.hpp"

#include <cmath>
#include <cassert>
#include <iostream>
#include <stdexcept>


BayesianClassifier::BayesianClassifier(std::vector<std::string> classes)
	: trained_(false)
	, totalWordsCount_(0)
	, countWordsForClass_()
	, classes_(std::move(classes))
	, samples_()
{
}

std::size_t BayesianClassifier::ClassesCount() const { return classes_.size(); }

std::size_t BayesianClassifier::SamplesCount() const{ return samples_.size(); }

bool BayesianClassifier::Trained() const { return trained_; }

void BayesianClassifier::Train(const std::vector<BayesianSample>& samples)
{
	samples_ = samples;
	countWordsForClass_.clear();
	countWordsForClass_.resize(ClassesCount());

	std::set<std::string> totalWords;
	for (auto&& sample : samples)
	{
		assert(sample.Class < ClassesCount());

		auto& classDict = countWordsForClass_.at(sample.Class);
		split_words(sample.Phrase, combine(WordsCounter(classDict), WordsSplitter(totalWords)));
	}

	totalWordsCount_ = totalWords.size();
	trained_ = true;
}

std::size_t BayesianClassifier::ClassOf(const std::string& newSample) const
{
	if (!Trained())
		throw std::runtime_error("BayesianClassifier is untrained");

	const std::set<std::string> newSampleWords = to_words_set(newSample);
	std::pair<double, std::size_t> maxEstimation(-std::numeric_limits<double>::infinity(), 0);
	for (std::size_t classID = 0; classID < ClassesCount(); ++classID)
	{
		const double estimation = CalculateEstimation(classID, newSampleWords);
		std::cout << "= " << estimation << '\n' << std::endl;

		if (estimation > maxEstimation.first)
			maxEstimation = { estimation, classID };
	}

	return maxEstimation.second;
}

std::string BayesianClassifier::Classify(const std::string& newSample) const
{
    return classes_.at(ClassOf(newSample));
}

std::size_t BayesianClassifier::SamplesOfClassCount(std::size_t classID) const
{
	return std::count_if(samples_.cbegin(), samples_.cend(), 
		[&](auto&& sample) { return sample.Class == classID; });
}

double BayesianClassifier::CalculateEstimation(std::size_t classID, const std::set<std::string>& newSampleWords) const
{
	assert(Trained());
	assert(classID < classes_.size());

	const auto& countsWordsInClass = countWordsForClass_.at(classID);
	const auto countDocumentsOfClass = SamplesOfClassCount(classID);
	std::cout << '[' << classes_.at(classID) << "]\n";
	std::cout << "log10(" << countDocumentsOfClass << " / " << samples_.size() << ")\n";

	double estimation = std::log10(countDocumentsOfClass / static_cast<double>(samples_.size()));
	for (const auto& word : newSampleWords)
	{
		const auto countOccurrencesOfWord = value_or_default(countsWordsInClass, word, 0);

		std::cout << "log10((" << ALPHA << " + " << countOccurrencesOfWord << ") / (" << totalWordsCount_ << " + " << countsWordsInClass.size() << ")) '" << word << "'\n";
		estimation += std::log10((ALPHA + countOccurrencesOfWord) / static_cast<double>(totalWordsCount_ + countsWordsInClass.size()));
	}

	return estimation;
}
