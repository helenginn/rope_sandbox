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

#ifndef __switch__CAlpha__
#define __switch__CAlpha__

#include <boost/shared_ptr.hpp>
#include <map>
#include <vector>
#include <hcsrc/vec3.h>
#include "Chain.h"

class Atom; typedef boost::shared_ptr<Atom> AtomPtr;
class AtomGroup; typedef boost::shared_ptr<AtomGroup> AtomGroupPtr;
class Whack; typedef boost::shared_ptr<Whack> WhackPtr;
class Bond; typedef boost::shared_ptr<Bond> BondPtr;

namespace Vagabond { class Crystal; }
typedef boost::shared_ptr<Vagabond::Crystal> CrystalPtr;

class CAlpha
{
public:
	CAlpha(AtomPtr ca, CrystalPtr c);

	void adjustFromReference(CAlpha *ca);
	void addToAverage(CAlpha *ca);
	void calculateAverages();
	
	CAlpha *getAlphaForCrystal(CrystalPtr c)
	{
		return _crystal2Alpha[c];
	}
	
	static double scoreBetweenAlphas(CAlpha *a, CAlpha *b, int type = 0);
	
	std::string displayName();
	
	CrystalPtr crystal()
	{
		return _c;
	}
	
	AtomPtr atom()
	{
		return _ca;
	}
	
	double phi()
	{
		return _phi;
	}
	
	double psi()
	{
		return _psi;
	}
	
	double deltaPhi(CAlpha *ref)
	{
		return _phi - ref->phi();
	}

	double deltaPsi(CAlpha *ref)
	{
		return _psi - ref->psi();
	}
	
	double referenceHeat(CAlpha *ref)
	{
		if (ref == NULL)
		{
			return _heat;
		}

		return ref->_heat;
	}
	
	double heat()
	{
		return _heat;
	}
	
	double myHeat()
	{
		return _myHeat;
	}
	
	void setHeat(double heat)
	{
		_heat = heat;
	}

	void contactHeat(AtomGroupPtr cas = AtomGroupPtr());
	void torsionHeat(CAlpha *ref);
	
	bool children()
	{
		return _alphas.size() > 0;
	}
	
	size_t alphaCount()
	{
		return _alphas.size();
	}
	
	void setOffset(vec3 off)
	{
		_offset = off;
	}
	
	vec3 &offset()
	{
		return _offset;
	}
	
	Chain *chain()
	{
		return _chain;
	}
	
	void setChain(Chain *c)
	{
		_chain = c;
	}
private:
	double fraction();
	AtomPtr _ca;
	CrystalPtr _c;
	BondPtr _parent;
	BondPtr _phiBond;
	BondPtr _psiBond;
	WhackPtr _whack;
	double _phi;
	double _psi;
	double _kPhi;
	double _kPsi;
	double _heat;
	double _myHeat;

	Chain *_chain;
	AtomGroupPtr _close;
	std::map<CrystalPtr, CAlpha *> _crystal2Alpha;
	std::vector<CAlpha *> _alphas;
	std::vector<CrystalPtr> _crystals;
	vec3 _offset;
};

#endif
