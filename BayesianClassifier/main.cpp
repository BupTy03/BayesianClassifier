#include "BayesianClassifier.hpp"

#include <iostream>


int main()
{
	setlocale(LC_ALL, "Russian");

	BayesianClassifier classifier({"Spam", "Ham"});
	classifier.Train({
		BayesianSample("Предоставляю услуги бухгалтера", 0),
		BayesianSample("Спешите купить iPhone", 0),
		BayesianSample("Надо купить молоко", 1)
	});

	const std::string answer = classifier.Classify("надо купить услуги iPhone");
	std::cout << "Answer: " << answer << std::endl;
	return 0;
}
