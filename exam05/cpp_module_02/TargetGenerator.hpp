#ifndef TARGETGENERATOR_HPP
# define TARGETGENERATOR_HPP

# include <vector>
# include "ATarget.hpp"

class TargetGenerator
{
private:
	std::vector<ATarget*> targets;
public:
	TargetGenerator();
	virtual ~TargetGenerator();

	void learnTargetType(ATarget *target);
	void forgetTargetType(std::string const &name);
	ATarget *createTarget(std::string const &name);
};

#endif
