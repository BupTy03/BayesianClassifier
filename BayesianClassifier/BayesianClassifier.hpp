#pragma once

#include <set>
#include <map>
#include <vector>
#include <string>


struct BayesianSample
{
	explicit BayesianSample(std::string phrase, std::size_t classID)
		: Phrase(std::move(phrase)), Class(classID) {}

	std::string Phrase;
	std::size_t Class;
};


class BayesianClassifier
{
public:
	static constexpr double ALPHA = 1.0;

public:
	explicit BayesianClassifier(std::vector<std::string> classes);

	std::size_t ClassesCount() const;
	std::size_t SamplesCount() const;

	bool Trained() const;
	void Train(const std::vector<BayesianSample>& samples);

	std::size_t ClassOf(const std::string& newSample) const;
	std::string Classify(const std::string& newSample) const;

	std::size_t SamplesOfClassCount(std::size_t classID) const;

private:
	double CalculateEstimation(std::size_t classID, const std::set<std::string>& newSampleWords) const;

private:
	bool trained_;
	std::size_t totalWordsCount_;
	std::vector<std::map<std::string, std::size_t>> countWordsForClass_;
	std::vector<std::string> classes_;
	std::vector<BayesianSample> samples_;
};

