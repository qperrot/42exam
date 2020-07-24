#include "TargetGenerator.hpp"

TargetGenerator::TargetGenerator() {}

TargetGenerator::~TargetGenerator() {
	std::vector<ATarget*>::iterator it = this->targets.begin();
	for (; it != this->targets.end(); ++it)
		delete *it;
}

void TargetGenerator::learnTargetType(ATarget *target)
{
	if (target)
	{
		std::vector<ATarget *>::iterator it = this->targets.begin();
		for (; it != this->targets.end(); ++it)
			if ((*it)->getType() == target->getType())
				return ;
		this->targets.push_back(target->clone());
	}
}

void TargetGenerator::forgetTargetType(std::string const &name) {
	std::vector<ATarget *>::iterator it = this->targets.begin();
	for (; it != this->targets.end(); ++it) {
		if ((*it)->getType() == name)
		{
			delete *it;
			it = this->targets.erase(it);
		}
	}
}

ATarget *TargetGenerator::createTarget(std::string const &name) {
	std::vector<ATarget *>::iterator it = this->targets.begin();
	for (; it != this->targets.end(); ++it)
	{
		if ((*it)->getType() == name) {
			return (*it);
		}
	}
	return (NULL);
}
