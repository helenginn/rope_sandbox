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
#include <vector>

class Chain;

class Entity
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

	std::string &name()
	{
		return _name;
	}
	
	void addChain(Chain *c)
	{
		_chains.push_back(c);
	}
	
	void setVisible(bool vis);
private:
	std::string _name;
	std::string _seq;
	std::vector<Chain *> _chains;
	int _tolerance;
};

#endif
