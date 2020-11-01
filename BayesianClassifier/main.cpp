#include "BayesianClassifier.hpp"

#include <iostream>


int main()
{
	setlocale(LC_ALL, "Russian");

	BayesianClassifier classifier({"Spam", "Ham"});
	classifier.Train({
		BayesianSample("������������ ������ ����������", 0),
		BayesianSample("������� ������ iPhone", 0),
		BayesianSample("���� ������ ������", 1)
	});

	const std::string answer = classifier.Classify("���� ������ ������ iPhone");
	std::cout << "Answer: " << answer << std::endl;
	return 0;
}
