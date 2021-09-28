// breathalyser
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

#ifndef __breathalyser__ensemble__ 
#define __breathalyser__ensemble__ 

#include <h3dsrc/SlipObject.h>
#include "Handleable.h"

class Text;
class Chain;
class Kabsch;
class Entity;
class Handle;
class CAlpha;
class AveCSV;
class Screen;
class Display;
class Composite;

class Atom; typedef boost::shared_ptr<Atom> AtomPtr;
class AtomGroup; typedef boost::shared_ptr<AtomGroup> AtomGroupPtr;
typedef std::vector<AtomPtr> AtomList;

/******************** Crystal definition ***************************/
namespace Vagabond
{
	class Crystal;
}

#define ToCrystalPtr(a) (boost::static_pointer_cast<Vagabond::Crystal>((a)))
typedef boost::shared_ptr<Vagabond::Crystal> CrystalPtr;
typedef boost::weak_ptr<Vagabond::Crystal> CrystalWkr;

/****************** end Crystal definition *************************/

class Ensemble : public SlipObject, public Handleable
{
public:
	Ensemble(Ensemble *parent, CrystalPtr c);

	void setDisplay(Display *d)
	{
		_display = d;
	}
	
	void setName(std::string name);
	
	size_t chainCount()
	{
		return _chains.size();
	}
	
	Chain *chain(int i)
	{
		return _chains[i];
	}
	
	Chain *chain(std::string ch)
	{
		if (_str2Chain.count(ch) == 0)
		{
			return NULL;
		}
		return _str2Chain[ch];
	}

	CrystalPtr crystal()
	{
		return _crystal;
	}
	
	void setReference(bool ref)
	{
		_isReference = ref;
	}
	
	void repopulate();
	void recolour();
	virtual std::string title();
	virtual void setTitle(std::string title);
	vec3 averagePos();
	
	Ensemble *ensemble(std::string name)
	{
		return _nameMap[name];
	}
	
	void setComposite(Composite *comp)
	{
		_composite = comp;
	}
	
	Composite *composite()
	{
		return _composite;
	}
	
	void setEntityAsChain(Entity *entity, std::string chain);

	Chain *chainObjectForEntity(Entity *entity, int i);
	
	Entity *entityForChain(std::string chain)
	{
		if (_chain2Entity.count(chain) == 0)
		{
			return NULL;
		}
		
		return _chain2Entity[chain];
	}
	
	std::string chainForEntity(Entity *entity, int i);
	
	void addEnsemble(Ensemble *e);
	
	void changeEnsembleName(Ensemble *e, std::string before,
	                        std::string after);
	
	size_t ensembleCount()
	{
		return _ensembles.size();
	}
	
	Ensemble *ensemble(int i)
	{
		return _ensembles[i];
	}

	virtual void render(SlipGL *gl);

	void minMaxResidues(std::string ch, int *min, int *max);
	std::string generateSequence(Chain *chain, int *minRes = NULL);
	vec3 centroidForChain(std::string chain);
	std::string findMatchingChain(std::string ch, Ensemble *other,
	                              bool onlyBlank = false);

	AveCSV *prepareCluster4x(Screen *scr);
	void prepareCluster4xPerAtom(Screen *scr);
	
	void takeEntityDefinition(std::string bit);
	
	void applyKabsch(Kabsch *k);
	
	virtual void setVisible(bool vis);
	
	virtual bool isVisible()
	{
		return !isDisabled();
	}
	
	static void setTextSize(int size)
	{
		_size = size;
	}
	
	void addCAlpha(CAlpha *c)
	{
		_calphas.push_back(c);
	}
	
	CAlpha *cAlpha(int i)
	{
		return _calphas[i];
	}
	
	size_t cAlphaCount()
	{
		return _calphas.size();
	}
	
	virtual void giveMenu(QMenu *m, Display *d);
	
	void defineEntity(Entity *entity, bool force = false);
	double torsionSimilarity(Ensemble *other, Ensemble *ref);
	double atomSimilarity(Ensemble *other, Ensemble *ref);
	double heatSimilarity(Ensemble *other, Ensemble *ref);
	double sequenceSimilarity(Ensemble *other);
	void chainsToHandle(Handle *h);
	Handle *duplicateHandle();
	
	void torsionHeat();
	void contactHeat();
	
	AtomGroupPtr cAlphaSubset()
	{
		return _cas;
	}
	
	std::string chainString();
	CAlpha *whichAtom(double x, double y);
	void deselectBalls();
	void selectBall(AtomPtr atom);
	void rejectNonEntities();
private:
	int mutationsForEntity(Ensemble *other, Entity *ent);

	std::vector<Text *> _texts;
	std::vector<Ensemble *> _ensembles;
	Ensemble *_parent;
	std::map<AtomPtr, Text *> _textMap;
	std::map<int, std::vector<std::string> > _muts;
	
	mat3x3 _rot;
	vec3 _trans, _target;

	void findChains();
	CrystalPtr _crystal;
	AtomGroupPtr _cas;
	std::vector<CAlpha *> _calphas;
	Display *_display;

	Composite *_composite;
	bool _isReference;
	static int _size;
	std::vector<Chain *> _chains;
	std::map<std::string, Entity *> _chain2Entity;
	std::map<Entity *, std::vector<std::string> > _entity2Chain;
	std::map<std::string, Chain *> _str2Chain;
	std::map<std::string, Ensemble *> _nameMap;
	CAlpha *_selectedAtom;
};

#endif
