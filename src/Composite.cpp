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

#include "Composite.h"
#include "Ensemble.h"
#include "Display.h"
#include "Kabsch.h"
#include "CAlpha.h"
#include "Handle.h"
#include "Entity.h"
#include "Chain.h"

#include <fstream>
#include <iomanip>
#include <libsrc/VBondReader.h>
#include <QThread>
#include <hcsrc/Timer.h>
#include <hcsrc/maths.h>
#include <libsrc/Crystal.h>
#include <libsrc/Atom.h>
#include <libsrc/Monomer.h>
#include <libsrc/Anchor.h>
#include <libsrc/Polymer.h>
#include <libsrc/Whack.h>
#include <libsrc/Bond.h>
#include <libsrc/Shouter.h>
#include <hcsrc/FileReader.h>
#include "MyDictator.h"

Composite::Composite() : QObject(NULL)
{
	_entity = NULL;
	_eRef = NULL;
	_chains.push_back("A");
	_dictator = NULL;
}

void Composite::loadFromFile(QString filename, QString handle)
{
	std::string contents = get_file_contents(filename.toStdString());
	
	loadModels(contents, handle.toStdString());
}

void Composite::loadModels(std::string list, std::string handle)
{
	std::vector<std::string> array = split(list, '\n');

	std::vector<CrystalPtr> current;
	CrystalPtr cryst;
	
	for (size_t i = 0; i < array.size(); i++)
	{
		std::vector<std::string> bits = split(array[i], ' ');
		std::string filename = bits[0];
		std::cout << "Loading " << i + 1 << " / " << 
		array.size() << ": " << filename << std::endl;
		VBondReader reader = VBondReader();
		reader.setFilename(filename);
		CrystalPtr crystal;
		try
		{
			crystal = reader.getCrystal();
			Options::getRuntimeOptions()->addCrystal(crystal);
			crystal->setSampleNum(1);

			_filenames[crystal] = filename;
			_f2Crystal[filename] = crystal;

			if (_crystals.size() == 0)
			{
				VBondReader reader = VBondReader();
				reader.setFilename(filename);
				_reference = reader.getCrystal();
				_reference->setSampleNum(1);
				_filenames[_reference] = filename;
				_eRef = _display->addCrystal(_reference, this);
			}
			
			if (handle.length() > 0 && current.size() == 0)
			{
				VBondReader reader = VBondReader();
				reader.setFilename(filename);
				cryst = reader.getCrystal();
				cryst->setSampleNum(1);
			}

			_crystals.push_back(crystal);
			current.push_back(crystal);
			Ensemble *e = _display->addCrystal(crystal, this, _eRef);
			e->setName(filename);
			
			for (size_t i = 1; i < bits.size(); i++)
			{
				e->takeEntityDefinition(bits[i]);
			}
			_ensembles[crystal] = e;
		}
		catch (Shouter *s)
		{
			std::string m = s->getMessage();
			std::cout << m << std::endl;
		}
	}
	
	Ensemble *extra = NULL;
	if (handle.length() > 0)
	{
		extra = _display->addCrystal(cryst, this);
		Handle *exh = extra->firstHandle();
		addEnsemble(extra);

		for (size_t i = 0; i < current.size(); i++)
		{
			Ensemble *same = _ensembles[current[i]];
			Handle *h = same->duplicateHandle();
			exh->addChild(h);
			extra->addEnsemble(same);
		}
	}

	for (size_t i = 0; i < _reference->moleculeCount(); i++)
	{
		if (_reference->molecule(i)->isPolymer())
		{
			AnchorPtr a = ToPolymerPtr(_reference->molecule(i))->getAnchorModel();
			ToPolymerPtr(_reference->molecule(i))->makeBackboneTwists(a);
		}
	}

	for (size_t j = 0; j < _crystals.size(); j++)
	{
		for (size_t i = 0; i < _crystals[j]->moleculeCount(); i++)
		{
			if (_crystals[j]->molecule(i)->isPolymer())
			{
				AnchorPtr a = ToPolymerPtr(_crystals[j]->molecule(i))->getAnchorModel();
				ToPolymerPtr(_crystals[j]->molecule(i))->makeBackboneTwists(a);
			}
		}
	}

	if (handle.length() > 0)
	{
		extra->setName(handle);
	}
	
	emit _eRef->changedName();
	
	emit resultReady();

}

void Composite::sortVisibility()
{
	for (size_t i = 0; i <= _crystals.size(); i++)
	{
		Ensemble *e = _eRef;
		if (i < _crystals.size())
		{
			e = _ensembles[_crystals[i]];
		}
		for (size_t j = 0; j < e->chainCount(); j++)
		{
			Entity *entity = e->chain(j)->entity();
			if (_visibility.count(entity))
			{
				bool show = _visibility[entity];
				e->chain(j)->setDisabled(!show);
			}
		}
	}

}

bool Composite::entityMustHide(Entity *entity)
{
	if (_visibility.count(entity))
	{
		return (!_visibility[entity]);
	}
	
	return false;
}

void Composite::hideEntity(std::string name)
{
	Entity *e = entity(name);
	if (e)
	{
		_visibility[e] = false;
	}

	sortVisibility();
}

void Composite::hideNonEntities(bool hide)
{
	_visibility[NULL] = false;
	sortVisibility();
}

void Composite::selectEntity(std::string entity)
{
	if (_name2Entity.count(entity))
	{
		_entity = _name2Entity[entity];
	}
	else
	{
		std::cout << "Could not find existing entity of name " << entity 
		<< std::endl;
	}
}

void Composite::defineEntity(std::string entity, std::string refChain)
{
	Entity *ent = new Entity(entity);
	_entities.push_back(ent);
	_name2Entity[entity] = ent;
	_eRef->setEntityAsChain(ent, refChain);
	_eRef->defineEntity(ent); // filling in any other matching chains

	for (size_t i = 0; i < _refs.size(); i++)
	{
		_refs[i]->defineEntity(ent);
	}

	for (size_t i = 0; i < _crystals.size(); i++)
	{
		Ensemble *e = _ensembles[_crystals[i]];
		e->defineEntity(ent);
	}
}

void Composite::align(QString entity_)
{
	std::string name = entity_.toStdString();
	Entity *ent = entity(name);
	std::string refChain = _eRef->chainForEntity(ent, 0);
	std::cout << "Aligning... ";
	if (ent)
	{
		std::cout << ent->name();
	}
	std::cout << std::endl;
	
	for (size_t i = 0; i < _refs.size(); i++)
	{
		Kabsch k(_reference, _refs[i]->crystal());
		Ensemble *e = _refs[i];
		std::string chain = e->chainForEntity(ent, 0);
		k.setChains(refChain, chain);
		k.run();
		
		e->applyKabsch(&k);
	}

	for (size_t i = 0; i < _crystals.size(); i++)
	{
		Kabsch k(_reference, _crystals[i]);

		Ensemble *e = _ensembles[_crystals[i]];
		std::string chain = e->chainForEntity(ent, 0);
		k.setChains(refChain, chain);

		k.run();
		
		e->applyKabsch(&k);
	}

	emit resultReady();
}

bool Composite::isValid(CrystalPtr c, AtomPtr a)
{
	// should loop, FIXME
	std::string chain = _ensembles[c]->chainForEntity(_entity, 0);
	if (a->getChainID().rfind(chain, 0) != 0)
	{
		return false;
	}

	if (a->getAtomName() != "CA")
	{
		return false;
	}
	
	if (a->getAlternativeConformer() != "A" && 
	    a->getAlternativeConformer() != "")
	{
		return false;
	}
	
	if (!a->getModel()->isBond())
	{
		return false;
	}
	
	return true;
}

void Composite::setReferenceTorsions(Ensemble *ref)
{
	if (!ref)
	{
		ref = _eRef;
	}

	if (ref && ref->cAlphaCount() > 0)
	{
		std::cout << ref->title() << " already has " << ref->cAlphaCount() 
		<< " atoms, skipping." << std::endl;
		return;
	}

	std::cout << "Resolving torsions from " << ref->title() << std::endl;

	CrystalPtr cryst = ref->crystal();
	cryst->refreshAnchors();

	for (size_t i = 0; i < cryst->moleculeCount(); i++)
	{
		if (!cryst->molecule(i)->isPolymer())
		{
			continue;
		}

		PolymerPtr pol = ToPolymerPtr(cryst->molecule(i));

		for (size_t j = pol->monomerBegin() + 2; j < pol->monomerEnd() - 2; j++)
		{
			if (!pol->getMonomer(j))
			{
				continue;
			}
			AtomPtr a = pol->getMonomer(j)->findAtom("CA");

			if (!isValid(cryst, a))
			{
				continue;
			}

			int resi = a->getResidueNum();
			std::string chain;
			chain += a->getChainID()[0];

			CAlpha *refAlpha = new CAlpha(a, cryst);
			ref->addCAlpha(refAlpha);
			_atom2Ca[a] = refAlpha;

			for (size_t k = 0; k < ref->ensembleCount(); k++)
			{
				Ensemble *e = ref->ensemble(k);
				AtomGroupPtr next = e->cAlphaSubset();
				CrystalPtr crystal = e->crystal();
				std::string match;
				Entity *entity;
				entity = ref->entityForChain(chain);
				if (entity != NULL)
				{
					match = e->chainForEntity(entity, 0);
				}
				else
				{
					match = ref->findMatchingChain(chain, e);
				}

				AtomList list = next->findAtoms("CA", resi, match);

				if (list.size() == 0)
				{
					continue;
				}
				
				if (_atom2Ca[list[0]] != NULL)
				{
					refAlpha->addToAverage(_atom2Ca[list[0]]);
					continue;
				}

				CAlpha *alpha = new CAlpha(list[0], crystal);
				e->addCAlpha(alpha);
				refAlpha->addToAverage(alpha);
				_atom2Ca[list[0]] = alpha;
			}

			refAlpha->calculateAverages();
			std::cout << a->getChainID()[0] << std::flush;
		}
	}

	_display->refreshEnsembles();
	
	std::cout << std::endl;
	std::cout << "Collected " << ref->cAlphaCount() << " cAlpha atoms" << std::endl;

	std::cout << std::endl;
}

void Composite::vectors(QString qfilename)
{
	std::string filename = qfilename.toStdString();

	if (_eRef->cAlphaCount() == 0)
	{
		setReferenceTorsions();
	}

	std::ofstream tfile;
	tfile.open(filename);

	for (size_t j = 0; j < _crystals.size(); j++)
	{
		std::string num = _filenames[_crystals[j]];
		tfile << num << ",";

		for (size_t i = 0; i < _eRef->cAlphaCount(); i++)
		{
			CAlpha *ref = _eRef->cAlpha(i);
			CAlpha *ca = ref->getAlphaForCrystal(_crystals[j]);
			
			double dPhi = 0; double dPsi = 0;
			
			if (ca)
			{
				dPhi = ca->deltaPhi(ref);
				dPsi = ca->deltaPsi(ref);

			}

			tfile << dPhi << "," << dPsi << ",";
		}
		
		tfile << std::endl;
	}

	tfile.close();

	emit resultReady();
}

void Composite::findCorrelations(std::string filename)
{
	if (_eRef->cAlphaCount() == 0)
	{
		setReferenceTorsions();
	}

	std::ofstream file;
	file.open(filename);
	std::cout << "Writing to " << filename << "..." << std::endl;

	for (size_t i = 0; i < _eRef->cAlphaCount() - 1; i++)
	{
		CAlpha *iRef = _eRef->cAlpha(i);

		for (size_t j = i; j < _eRef->cAlphaCount(); j++)
		{
			CAlpha *jRef = _eRef->cAlpha(i);

			double score = CAlpha::scoreBetweenAlphas(iRef, jRef);
			std::string anum = iRef->displayName();
			std::string bnum = jRef->displayName();
			file << anum << "," << bnum << "," << score << std::endl;
		}
	}
	
	file.close();
	std::cout << "Done" << std::endl;
}

void Composite::setChains(std::string chains)
{
	_chains = split(chains, ',');
}

void Composite::setDisplay(Display *d)
{
	_display = d;
}

void Composite::torsionHeatForEnsemble(Ensemble *ref)
{
	std::cout << "Torsion heat!" << std::endl;
	if (ref == NULL)
	{
		ref = _eRef;
	}
	if (ref->cAlphaCount() == 0)
	{
		setReferenceTorsions(ref);
	}

	for (size_t i = 0; i < ref->cAlphaCount(); i++)
	{
		CAlpha *a = ref->cAlpha(i);
		int j = 0;
		std::string chain = ref->chainForEntity(_entity, j);
		while (chain.length())
		{
			if (_entity == NULL || 
			    a->atom()->getChainID().rfind(chain, 0) == 0)
			{
				a->torsionHeat(a);
			}
			j++;
			chain = ref->chainForEntity(_entity, j);
		}
	}

	_display->colourEnsembles();
	emit resultReady();
}

void Composite::torsionHeat()
{
	torsionHeatForEnsemble(_eRef);
}

void Composite::contactHeatForEnsemble(Ensemble *e)
{
	if (e->cAlphaCount() == 0)
	{
		setReferenceTorsions(e);
	}
	
	CrystalPtr cryst = e->crystal();
	AtomList atoms = cryst->findAtoms("CA");
	AtomGroupPtr casOnly = AtomGroupPtr(new AtomGroup());
	casOnly->addAtomsFrom(atoms);
	casOnly->setTop(cryst);

	for (size_t i = 0; i < e->cAlphaCount(); i++)
	{
		CAlpha *a = e->cAlpha(i);
		int j = 0;
		std::string chain = e->chainForEntity(_entity, j);
		while (chain.length())
		{
			if (a->atom()->getChainID().rfind(chain, 0) == 0)
			{
				a->contactHeat();
			}
			j++;
			chain = e->chainForEntity(_entity, j);
		}
	}

	_display->colourEnsembles();
}

void Composite::contactHeat()
{
	std::cout << "Contact heat!" << std::endl;
	contactHeatForEnsemble(_eRef);
	
	std::ofstream file;
	file.open("heat.csv");

	for (size_t i = 0; i < _eRef->cAlphaCount(); i++)
	{
		CAlpha *a = _eRef->cAlpha(i);
		int j = 0;
		std::string chain = _eRef->chainForEntity(_entity, j);
		while (chain.length())
		{
			if (a->atom()->getChainID().rfind(chain, 0) == 0)
			{
				file << a->displayName() << "," << a->heat();
				file << std::endl;
			}
			j++;
			chain = _eRef->chainForEntity(_entity, j);
		}
	}
	
	file.close();
	
	emit resultReady();
}

void Composite::writePDB(std::string suffix)
{
	for (size_t i = 0; i < _eRef->cAlphaCount(); i++)
	{
		CAlpha *a = _eRef->cAlpha(i);
		double heat = a->heat();
		AtomPtr ca = a->atom();
		int resi = ca->getResidueNum();
		MoleculePtr mol = ca->getMolecule();
		
		if (!mol->isPolymer())
		{
			return;
		}
		
		PolymerPtr pol = ToPolymerPtr(mol);
		MonomerPtr mon = pol->getMonomer(resi);
		
		for (size_t j = 0; j < mon->atomCount(); j++)
		{
			AtomPtr b = mon->atom(j);
			b->setInitialBFactor(heat);
		}
	}
	
	_reference->makePDBs(suffix);
}

void Composite::heatFromCSV(std::string filename)
{
	std::string refChain = _eRef->chainForEntity(_entity, 0);

	std::string contents = get_file_contents(filename);
	std::vector<std::string> lines = split(contents, '\n');
	if (_eRef->cAlphaCount() == 0)
	{
		setReferenceTorsions();
	}

	for (size_t i = 0; i < lines.size(); i++)
	{
		std::string line = lines[i];
		std::vector<std::string> bits = split(line, ',');

		if (bits.size() < 2)
		{
			continue;
		}
		
		trim(bits[0]); trim(bits[1]);
		int resi = atoi(bits[0].c_str());
		double heat = atof(bits[1].c_str());
		std::cout << resi << " " << heat << " " << log(heat) << std::endl;
		if (heat > 0)
		{
			heat = log(heat);
		}

		if (heat != heat) continue;
		
		AtomList list = _reference->findAtoms("CA", resi, refChain);
		
		for (size_t i = 0; i < list.size(); i++)
		{
			CAlpha *a = _atom2Ca[list[i]];
			if (a)
			{
				a->setHeat(heat);
			}
		}
	}

	_display->colourEnsembles();
	std::cout << "Done" << std::endl;
}

void Composite::addEnsemble(Ensemble *e)
{
	_refs.push_back(e);
	_ensembles[e->crystal()] = e;
}
