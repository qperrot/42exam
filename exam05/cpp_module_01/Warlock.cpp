#include "Warlock.hpp"

Warlock::Warlock(std::string const &name, std::string const &title):
	name(name), title(title) {
	std::cout << this->name << ": What a boring day\n";
}

Warlock::~Warlock() {
	std::cout << this->name << ": My job here is done!\n";
	std::vector<ASpell*>::iterator it = this->spells.begin();
	for (; it != this->spells.end(); ++it)
		delete *it;
	// this->spells.clear();
}

std::string const &Warlock::getName(void) const {
	return (this->name);
}

std::string const &Warlock::getTitle(void) const {
	return (this->title);
}

void Warlock::setTitle(std::string const &title) {
	this->title = title;
}

void Warlock::introduce(void) const {
	std::cout << this->name << ": My name is " << this->name << ", " << this->title << "!\n";
}

void Warlock::learnSpell(ASpell *spell) {
	if (spell) {
		std::vector<ASpell *>::iterator it = this->spells.begin();
		for (; it != this->spells.end(); ++it)
			if ((*it)->getName() == spell->getName())
				return;
		this->spells.push_back(spell->clone());
	}
}

void Warlock::forgetSpell(std::string const &spellName) {
	std::vector<ASpell *>::iterator it = this->spells.begin();
	for (; it != this->spells.end(); ++it)
	{
		if ((*it)->getName() == spellName) {
			delete *it;
			this->spells.erase(it);
			return;
		}
	}
}

void Warlock::launchSpell(std::string const &spellName, ATarget const &target) {
	std::vector<ASpell *>::iterator it = this->spells.begin();
	for (; it != this->spells.end(); ++it)
	{
		if ((*it)->getName() == spellName) {
			(*it)->launch(target);
			return ;
		}
	}
}
