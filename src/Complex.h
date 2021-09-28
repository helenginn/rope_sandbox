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

#ifndef __switch__Complex__
#define __switch__Complex__

#include <string>
#include <vector>
#include <hcsrc/vec3.h>
#include "Collective.h"

class Ensemble;
class Chain;
class Entity;

class Complex : public Collective
{
public:
	Complex(std::string name);

	void provideExample(Ensemble *e, std::string chains);
	virtual std::string findChainsInEnsemble(Ensemble *e);
	
	virtual std::string title()
	{
		return _name;
	}
	
	virtual bool hasChain(Chain *ch);
	
	virtual void setVisible(bool);
private:
	std::string _name;
	/** Just reference chains for nominal example */
	std::vector<Chain *> _chains;
	std::vector<Entity *> _entities;
	std::vector<vec3> _positions;
	std::vector<int> _defaultOffsets;
	
	/** all chains across all instances of complex */
	std::vector<Chain *> _members;
};

#endif
