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

#ifndef __switch__Collective__
#define __switch__Collective__

#include <boost/shared_ptr.hpp>
#include <map>
#include "Handleable.h"

namespace Vagabond { class Crystal; }
typedef boost::shared_ptr<Vagabond::Crystal> CrystalPtr;

class Chain;
class Ensemble;
class Composite;

class Collective : public Handleable
{
public:
	Collective();
	virtual ~Collective() {};
	
	void setComposite(Composite *c)
	{
		_composite = c;
	}
	
	virtual bool hasChain(Chain *ch) = 0;

	virtual std::string findChainsInEnsemble(Ensemble *e) = 0;
	std::string findChainsInCrystal(CrystalPtr c);
	std::string findChainInCrystal(CrystalPtr c, std::string ref);
	int offsetForCrystalChain(CrystalPtr c, std::string ch, int residue);
	
	virtual bool isVisible()
	{
		return _visible;
	}
protected:
	std::map<Ensemble *, std::string> _results;
	std::map<Chain *, int> _offsets;
	bool _visible;
	Ensemble *_refEnsemble;

private:
	Composite *_composite;
};

#endif
