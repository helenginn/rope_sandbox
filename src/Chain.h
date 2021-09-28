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
#include <hcsrc/Blast.h>
#include "Handleable.h"

namespace Vagabond { class Crystal; }
typedef boost::shared_ptr<Vagabond::Crystal> CrystalPtr;

class Icosahedron;
class Collective;
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
	
	bool belongsToCollective(Collective *c);
	
	Entity *entity()
	{
		return _entity;
	}
	
	std::string str()
	{
		return _chain;
	}
	
	Ensemble *ensemble()
	{
		return _ensemble;
	}
	
	void setSequence(std::string seq)
	{
		_sequence = seq;
	}
	
	std::string sequence()
	{
		return _sequence;
	}
	
	void setHide(bool val)
	{
		_visibilityDemand = !val;
	}
	
	virtual bool isVisible()
	{
		return !isDisabled();
	}
	
	void setComposite(Composite *comp)
	{
		_composite = comp;
	}
	
	int offsetFromChain(Chain *c);
	CAlpha *whichAtom(double x, double y, double *z);
	
	virtual void setVisible(bool vis);
	virtual void render(SlipGL *gl);
	void deselectBalls();
	void selectBall(AtomPtr atom);
	void correlationToCAlpha(Ensemble *ref, CAlpha *target);
	
	size_t offsetCount()
	{
		return _perResOffset.size();
	}

	int offsetForRefResidue(int i);
private:
	void convertToCylinder();
	void addCircle(vec3 centre, std::vector<vec3> &circle);
	void addCylinderIndices(size_t num);
	void convertToBezier();
	void addCAlpha(AtomPtr ca, vec3 point);
	void heatToVertex(Helen3D::Vertex &v, double heat);
	void correlationToVertex(Helen3D::Vertex &v, double cc);

	std::string _chain;
	std::string _sequence;
	/* from reference to my sequence */
	std::vector<int> _perResOffset;
	int _globalOffset;
	Entity *_entity;
	Composite *_composite;
	CrystalPtr _crystal;
	Ensemble *_ensemble;

	Alignment _a, _b;
	int _vPerAtom;
	std::map<AtomPtr, CAlpha *> _caMap;
	std::vector<AtomPtr> _cas;
	std::map<AtomPtr, size_t> _ca2Vertex;
	std::map<Icosahedron *, AtomPtr> _ball2Atom;
	std::map<AtomPtr, Icosahedron *> _atom2Ball;
	bool _visibilityDemand;
	std::vector<Icosahedron *> _balls;
};

#endif
