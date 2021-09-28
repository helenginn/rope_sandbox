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

#include <boost/shared_ptr.hpp>
#include <hcsrc/vec3.h>
#include <QObject>

class MyDictator;
class Collective;
class Entity;
class SVDBond;
class Display;
class CAlpha;
class Complex;

namespace Vagabond { class Crystal; }
typedef boost::shared_ptr<Vagabond::Crystal> CrystalPtr;

class Atom;
class Ensemble;
typedef boost::shared_ptr<Atom> AtomPtr;

class Composite : public QObject
{
Q_OBJECT
public:
	Composite();
	
	void setReferenceTorsions(Ensemble *ref = NULL);
	void findCorrelations(std::string filename);
	void loadModels(std::string list, std::string handle = "");
	void defineEntity(std::string entity, std::string refChain);
	void defineComplex(std::string name, std::string chains);
	
	void setChains(std::string chain);
	void hideNonEntities(bool hide);
	void writePDB(std::string last);
	void heatFromCSV(std::string filename);
	
	void setDisplay(Display *d);

	CAlpha *getCAlpha(AtomPtr a)
	{
		return _atom2Ca[a];
	}
	
	std::string nameForCrystal(CrystalPtr c)
	{
		return _filenames[c];
	}
	
	size_t entityCount()
	{
		return _entities.size();
	}
	
	Entity *entity(std::string name)
	{
		if (_name2Entity.count(name) == 0)
		{
			return NULL;
		}
		
		return _name2Entity[name];
	}
	
	Complex *getComplex(std::string name)
	{
		if (_name2Complex.count(name) == 0)
		{
			return NULL;
		}
		
		return _name2Complex[name];
	}
	
	Entity *chosenEntity()
	{
		return _entity;
	}
	
	Entity *entity(int i)
	{
		return _entities[i];
	}
	
	Ensemble *ensembleForCrystal(CrystalPtr c)
	{
		if (_ensembles.count(c))
		{
			return _ensembles[c];
		}
		
		return NULL;
	}
	
	void addEnsemble(Ensemble *e);
	
	void selectCollective(std::string name);
	
	void testCCA();
	void hideEntity(std::string name);
	void rejectOthers();
	void defineEntity(std::string entity);
	void torsionHeatForEnsemble(Ensemble *ref);
	void contactHeatForEnsemble(Ensemble *e);
	
	Collective *collective()
	{
		return _collective;
	}
public slots:
	void loadFromFile(QString filename, QString handle = "");
	void vectors(QString filename);
	void align(QString entity);
	void contactHeat();
	void torsionHeat();
signals:
	void resultReady();
private:
	void sortVisibility();
	bool isValid(CrystalPtr c, AtomPtr a);

	Ensemble *_eRef;
	std::vector<Ensemble *> _refs;
	MyDictator *_dictator;

	Display *_display;

	CrystalPtr _reference;
	std::vector<std::string> _chains;

	std::vector<CAlpha *> _cas;
	std::map<AtomPtr, CAlpha *> _atom2Ca;

	std::map<CrystalPtr, std::string> _filenames;
	std::map<CrystalPtr, Ensemble *> _ensembles;
	std::map<std::string, CrystalPtr> _f2Crystal;
	std::map<std::string, CrystalPtr> _name2Crystal;

	std::vector<CrystalPtr> _crystals;
	std::vector<Entity *> _entities;
	std::map<std::string, Entity *> _name2Entity;
	std::map<Entity *, bool> _visibility;
	Collective *_collective;
	Entity *_entity;
	std::vector<Complex *> _complexes;
	std::map<std::string, Complex *> _name2Complex;
};
