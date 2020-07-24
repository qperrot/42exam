#ifndef ATARGET_HPP
# define ATARGET_HPP

# include <iostream>
# include <string>

class ASpell;

class ATarget
{
protected:
	std::string type;
public:
	ATarget();
	ATarget(std::string const &type);
	ATarget(ATarget const &other);
	virtual ~ATarget();

	ATarget &operator=(ATarget const &other);

	std::string const &getType(void) const;

	void getHitBySpell(ASpell const &spell) const;

	virtual ATarget *clone(void) const = 0;
};

# include "ASpell.hpp"

#endif
