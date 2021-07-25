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

#ifndef __switch__Chain__
#define __switch__Chain__

#include <h3dsrc/SlipObject.h>
#include "Handleable.h"

namespace Vagabond { class Crystal; }
typedef boost::shared_ptr<Vagabond::Crystal> CrystalPtr;

class Composite;
class Ensemble;
class Entity;
class CAlpha;
class Atom;
typedef boost::shared_ptr<Atom> AtomPtr;

class Chain : public SlipObject, public Handleable
{
public:
	Chain(CrystalPtr cryst, std::string chain);

	void repopulate(mat3x3 r = make_mat3x3(),
	                vec3 t = empty_vec3(), vec3 loc = empty_vec3());
	
	void getAlphas(Composite *c);
	void recolour();
	
	virtual std::string title();
	
	void setEnsemble(Ensemble *e)
	{
		_ensemble = e;
	}
	
	void setEntity(Entity *entity);
	
	Entity *entity()
	{
		return _entity;
	}
	
	std::string str()
	{
		return _chain;
	}
	
	void setSequence(std::string seq)
	{
		_sequence = seq;
	}
	
	std::string sequence()
	{
		return _sequence;
	}
	
	virtual bool isVisible()
	{
		return !isDisabled();
	}
	
	virtual void setVisible(bool vis);
private:
	void convertToCylinder();
	void addCircle(vec3 centre, std::vector<vec3> &circle);
	void addCylinderIndices(size_t num);
	void convertToBezier();
	void addCAlpha(AtomPtr ca, vec3 point);
	void heatToVertex(Helen3D::Vertex &v, double heat);

	std::string _chain;
	std::string _sequence;
	Entity *_entity;
	CrystalPtr _crystal;
	Ensemble *_ensemble;

	int _vPerAtom;
	std::map<AtomPtr, CAlpha *> _caMap;
	std::vector<AtomPtr> _cas;
	std::map<AtomPtr, size_t> _ca2Vertex;
};

#endif
