// switch
// Copyright (C) 2019 Helen Ginn
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Please email: vagabond @ hginn.co.uk for more details.

#ifndef __switch__Entity__
#define __switch__Entity__

#include <string>
#include <algorithm>
#include <vector>
#include "Collective.h"

class Chain;
class Ensemble;

class Entity : public Collective
{
public:
	Entity(std::string name);
	
	std::string sequence()
	{
		return _seq;
	}
	
	int tolerance()
	{
		return _tolerance;
	}

	void setSequence(std::string seq)
	{
		_seq = seq;
	}
	
	void setSequenceFromChain(Chain *c);
	virtual std::string findChainsInEnsemble(Ensemble *e);

	std::string &name()
	{
		return _name;
	}
	
	virtual std::string title();
	
	void addChain(Chain *c)
	{
		if (_chains.size() == 0)
		{
			_reference = c;
			sortReference();
		}
		_chains.push_back(c);
	}
	
	bool hasChain(Chain *ch)
	{
		return std::find(_chains.begin(), _chains.end(), ch) != _chains.end();
	}
	
	void sortReference();
	
	virtual void setVisible(bool vis);
private:
	std::string _name;
	std::string _seq;
	std::vector<Chain *> _chains;
	Chain *_reference;
	int _tolerance;
	int _refMinRes;
};

#endif
