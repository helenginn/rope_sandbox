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

#ifndef __switch__Kabsch__
#define __switch__Kabsch__

#include <boost/shared_ptr.hpp>
#include <hcsrc/mat3x3.h>
#include <vector>
#include <string>
#include <map>

namespace Vagabond { class Crystal; }
typedef boost::shared_ptr<Vagabond::Crystal> CrystalPtr;

class Atom;
typedef boost::shared_ptr<Atom> AtomPtr;

class Composite;

class Kabsch
{
public:
	Kabsch(CrystalPtr ref, CrystalPtr move);

	void setChains(std::string ref, std::string move)
	{
		_refChain = ref;
		_moveChain = move;
	}
	
	const mat3x3 &rotation()
	{
		return _mrot;
	}
	
	const vec3 &translation()
	{
		return _trans;
	}

	const vec3 &target()
	{
		return _target;
	}

	void run();
private:
	bool correctChain(std::string id);
	CrystalPtr _ref;
	CrystalPtr _move;

	mat3x3 _mrot;
	vec3 _trans;
	vec3 _target;
	std::string _refChain, _moveChain;
	std::map<AtomPtr, AtomPtr> _caMap;
	std::vector<AtomPtr> _cas;
};

#endif
