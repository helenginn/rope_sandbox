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

#include "Ensemble.h"
#include "Entity.h"
#include "Display.h"
#include "Chain.h"
#include "Kabsch.h"
#include "Handle.h"
#include "CAlpha.h"
#include "Composite.h"

#include <libsrc/PDBReader.h>
#include <libsrc/Crystal.h>
#include <libsrc/Absolute.h>
#include <c4xsrc/ClusterList.h>
#include <c4xsrc/AveCSV.h>
#include <c4xsrc/Group.h>
#include <h3dsrc/Icosahedron.h>
#include <h3dsrc/Text.h>
#include <iostream>
#include <algorithm>
#include <QMenu>
#include <libsrc/Polymer.h>
#include <libsrc/Monomer.h>
#include <libsrc/Atom.h>
#include <libinfo/GeomTable.h>
#include <hcsrc/Blast.h>

int Ensemble::_size = 120;

Ensemble::Ensemble(Ensemble *parent, CrystalPtr c) : SlipObject(), Handleable()
{
	_crystal = c;
	_cas = AtomGroupPtr(new AtomGroup());
	_cas->addAtomsFrom(_crystal->findAtoms("CA"));
	_display = NULL;
	
	_rot = make_mat3x3();
	_trans = empty_vec3();
	_target = empty_vec3();

	_parent = parent;
	if (_parent)
	{
		_parent->addEnsemble(this);
	}
	_selected = NULL;
	_renderType = GL_LINES;
	_isReference = false;
	findChains();
	setName("Ensemble");
}

void Ensemble::setName(std::string n)
{
	std::string old = name();
	SlipObject::setName(n);
	if (_parent)
	{
		_parent->changeEnsembleName(this, old, n);
	}

	Handleable::changeName(n);
}

std::string Ensemble::title()
{
	std::string prep;
	if (_isReference)
	{
		prep = "all (" + i_to_str(ensembleCount()) + ")";
		return prep;
	}
	prep += name();
	return prep;
}

vec3 Ensemble::centroidForChain(std::string chain)
{
	Chain *ch = _str2Chain[chain];

	return ch->centroid();
}

std::string Ensemble::generateSequence(Chain *chain, int *minRes)
{
	if (chain->sequence().length())
	{
		return chain->sequence();
	}

	std::map<int, std::string> resMap;
	AtomList atoms = _crystal->findAtoms("CA", INT_MAX, chain->str());
	
	int min = INT_MAX;
	int max = -INT_MAX;

	for (size_t i = 0; i < atoms.size(); i++)
	{
		AtomPtr a = atoms[i];
		
		if (!a->getMonomer())
		{
			continue;
		}

		int resi = a->getResidueNum();
		std::string tlc = a->getMonomer()->getIdentifier();
		std::string code = GeomTable::getResCode(tlc); 
		
		resMap[resi] = code;
		
		if (resi < min)
		{
			min = resi;
		}
		
		if (resi > max)
		{
			max = resi;
		}
	}

	std::string seq;
	for (int i = min; i <= max; i++)
	{
		if (resMap.count(i))
		{
			seq += resMap[i];
		}
		else
		{
			seq += " ";
		}
	}
	
	if (minRes != NULL)
	{
		*minRes = min;
	}
	
	chain->setSequence(seq);
	
	return seq;
}

void Ensemble::findChains()
{
	if (!_crystal)
	{
		return;
	}
	
	std::vector<std::string> all;

	for (size_t i = 0; i < _crystal->moleculeCount(); i++)
	{
		MoleculePtr m = _crystal->molecule(i);
		
		if (!m->isPolymer())
		{
			continue;
		}

		all.push_back(m->getChainID());
	}
	
	for (size_t i = 0; i < _chains.size(); i++)
	{
		delete _chains[i];
	}

	_chains.clear();
	_str2Chain.clear();
	std::vector<std::string> unique;
	
	for (size_t i = 0; i < all.size(); i++)
	{
		std::string first;
		first += all[i][0];
		
		if (std::find(unique.begin(), unique.end(), first) 
		    != unique.end())
		{
			continue;
		}

		unique.push_back(first);
	}
	
	for (size_t i = 0; i < unique.size(); i++)
	{
		Chain *ch = new Chain(_crystal, unique[i]);
		ch->setEnsemble(this);
		std::string seq = generateSequence(ch);
		_chains.push_back(ch);
		_str2Chain[unique[i]] = ch;
	}
}

void Ensemble::minMaxResidues(std::string ch, int *min, int *max)
{
	*min = INT_MAX;
	*max = -INT_MAX;

	for (size_t i = 0; i < _crystal->moleculeCount(); i++)
	{
		MoleculePtr m = _crystal->molecule(i);
		
		if (!m->isPolymer())
		{
			continue;
		}

		if (m->getChainID().substr(0, ch.length()) == ch)
		{
			PolymerPtr p = ToPolymerPtr(m);
			if (p->monomerBegin() < *min)
			{
				*min = p->monomerBegin();
			}
			else if (p->monomerEnd() > *max)
			{
				*max = p->monomerEnd();
			}
		}
	}
}

void Ensemble::setVisible(bool vis)
{
	for (size_t i = 0; i < _ensembles.size(); i++)
	{
		_ensembles[i]->setVisible(vis);
	}

	for (size_t i = 0; i < _chains.size(); i++)
	{
		_chains[i]->setVisible(vis);
	}

	setDisabled(!vis);
}

void Ensemble::recolour()
{
	for (size_t i = 0; i < _ensembles.size(); i++)
	{
		_ensembles[i]->recolour();
	}

	for (size_t i = 0; i < _chains.size(); i++)
	{
		_chains[i]->recolour();
	}
}

void Ensemble::repopulate()
{
	for (size_t i = 0; i < _ensembles.size(); i++)
	{
		_ensembles[i]->repopulate();
	}

	for (size_t i = 0; i < _chains.size(); i++)
	{
		_chains[i]->repopulate(_rot, _trans, _target);
		_chains[i]->getAlphas(_composite);
		_chains[i]->recolour();
	}
}

vec3 Ensemble::averagePos()
{
	vec3 sum = empty_vec3();
	for (int i = 0; i < chainCount(); i++)
	{
		vec3 p = _chains[i]->centroid();
		if (!vec3_is_sane(p))
		{
			continue;
		}

		sum += p;
	}
	
	vec3_mult(&sum, 1 / (double)chainCount());
	
	if (!vec3_is_sane(sum))
	{
		return empty_vec3();
	}
	
	return sum;
}

void Ensemble::render(SlipGL *gl)
{
	for (int i = 0; i < ensembleCount(); i++)
	{
		_ensembles[i]->render(gl);
	}
	
	if (ensembleCount() == 0)
	{
		for (size_t i = 0; i < chainCount(); i++)
		{
			_chains[i]->render(gl);
		}

		SlipObject::render(gl);
	}
}

std::string Ensemble::findMatchingChain(std::string ch, Ensemble *other,
                                        bool onlyBlank)
{
	Chain *chain = _str2Chain[ch];
	if (chain == NULL)
	{
		return "";
	}
	std::string seq = generateSequence(chain);
	int best_mut = INT_MAX;
	double best_length = FLT_MAX;
	Chain *best_ch = NULL;
	vec3 this_c = centroidForChain(ch);

	for (size_t i = 0; i < other->chainCount(); i++)
	{
		Chain *other_ch = other->chain(i);
		if (onlyBlank && other_ch->entity() != NULL)
		{
			continue;
		}
		int muts, dels;
		std::string otherseq = other->generateSequence(other_ch);
		compare_sequences(seq, otherseq, &muts, &dels);
		vec3 diff = other->centroidForChain(other_ch->str());
		vec3_subtract_from_vec3(&diff, this_c);
		double l = vec3_length(diff);
		
		if (muts < best_mut)
		{
			best_ch = other_ch;
			best_mut = muts;
			best_length = l;
		}
		else if (muts <= (int)seq.length() / 1.5)
		{
			vec3 diff = other->centroidForChain(other_ch->str());
			vec3_subtract_from_vec3(&diff, this_c);
			double l = vec3_length(diff);

			if (l < best_length)
			{
				best_ch = other_ch;
				best_mut = muts;
				best_length = l;
			}

		}
	}
	
	if (best_ch && best_mut > (int)seq.length() / 2)
	{
		std::cout << "Comparing " << other->name() << " to " 
		<< name() << std::endl;
		std::cout << best_mut << " mutations, ";
		std::cout << best_length << " Å separation." << std::endl;
		std::cout << "Best match for chain " << ch << " is " 
		<< best_ch->str() << std::endl;
		std::cout << "Warning: poor identity with best sequence!" << std::endl;
		std::cout << std::endl;
	}
	
	if (best_ch == NULL)
	{
		return "";
	}

	return best_ch->str();
}

void Ensemble::applyKabsch(Kabsch *k)
{
	_rot = k->rotation();
	_rot = mat3x3_transpose(_rot);
	_trans = k->translation();
	_target = k->target();
	repopulate();
}

void Ensemble::chainsToHandle(Handle *h)
{
	for (size_t i = 0; i < chainCount(); i++)
	{
		Handle *sub = new Handle(_chains[i], h);
		h->addChild(sub);
	}
}

void Ensemble::setEntityAsChain(Entity *entity, std::string chain)
{
	_chain2Entity[chain] = entity;
	_entity2Chain[entity].push_back(chain);
	if (_str2Chain.count(chain) > 0)
	{
		_str2Chain[chain]->setEntity(entity);
		
		if (entity->sequence().length())
		{
			entity->setSequenceFromChain(_str2Chain[chain]);
		}
	}
}

Handle *Ensemble::duplicateHandle()
{
	Handle *dup = new Handle(this, NULL);
	chainsToHandle(dup);
	return dup;
}

double Ensemble::sequenceSimilarity(Ensemble *other)
{
	Entity *current = _composite->chosenEntity();

	Chain *myChain = chainObjectForEntity(current, 0);
	std::string myseq = myChain->sequence();

	Chain *herChain = other->chainObjectForEntity(current, 0);
	std::string herseq = herChain->sequence();

	int muts, dels;
	compare_sequences(myseq, herseq, &muts, &dels);

	return (double)muts / 20.;
}

double Ensemble::heatSimilarity(Ensemble *other, Ensemble *ref)
{
	CorrelData cd = empty_CD();
	
	for (int i = 0; i < ref->cAlphaCount(); i++)
	{
		CAlpha *r = ref->cAlpha(i);
		CAlpha *ca = r->getAlphaForCrystal(crystal());
		CAlpha *cb = r->getAlphaForCrystal(other->crystal());

		if (!ca || !cb || !r)
		{
			continue;
		}

		double aHeat = ca->myHeat();
		double bHeat = cb->myHeat();

		add_to_CD(&cd, aHeat, bHeat);
	}

	return evaluate_CD(cd);

}

double Ensemble::torsionSimilarity(Ensemble *other, Ensemble *ref)
{
	CorrelData cd = empty_CD();
	
	for (int i = 0; i < ref->cAlphaCount(); i++)
	{
		CAlpha *r = ref->cAlpha(i);
		CAlpha *ca = r->getAlphaForCrystal(crystal());
		CAlpha *cb = r->getAlphaForCrystal(other->crystal());

		if (!ca || !cb || !r)
		{
			continue;
		}

		double rPhi = ca->deltaPhi(r);
		double rPsi = ca->deltaPsi(r);

		double dPhi = cb->deltaPhi(r);
		double dPsi = cb->deltaPsi(r);

		add_to_CD(&cd, rPhi, dPhi);
		add_to_CD(&cd, dPsi, rPsi);
	}

	return evaluate_CD(cd);
}

AveCSV *Ensemble::prepareCluster4x(Screen *scr)
{
	std::cout << "cluster4x for " << title() << std::endl;
	AveCSV *csv = new AveCSV(NULL, "");
	ClusterList *list = scr->getList();
	csv->setList(list);
	if (csv->csvCount() == 0)
	{
		csv->startNewCSV("torsions");
		csv->startNewCSV("heat");
		csv->startNewCSV("sequence");
	}

	int total = 0;
	for (size_t i = 0; i < ensembleCount() - 1; i++)
	{
		for (size_t j = i + 1; j < ensembleCount(); j++)
		{
			double cc = _ensembles[i]->torsionSimilarity(_ensembles[j], this);
			double heat = _ensembles[i]->heatSimilarity(_ensembles[j], this);
			double seq = _ensembles[i]->sequenceSimilarity(_ensembles[j]);
			
			csv->setChosen(0);
			csv->addValue(_ensembles[i]->name(), _ensembles[j]->name(), cc);
			csv->addValue(_ensembles[j]->name(), _ensembles[i]->name(), cc);
			csv->setChosen(1);
			csv->addValue(_ensembles[i]->name(), _ensembles[j]->name(), heat);
			csv->addValue(_ensembles[j]->name(), _ensembles[i]->name(), heat);
			csv->setChosen(2);
			csv->addValue(_ensembles[j]->name(), _ensembles[i]->name(), seq);
			csv->addValue(_ensembles[i]->name(), _ensembles[j]->name(), seq);
			total++;
		}
	}

	std::cout << "Total " << total << " comparisons." << std::endl;

	csv->preparePaths();
	csv->setChosen(0);

	Group *top = Group::topGroup();
	top->setCustomName(name());
	top->updateText();
	
	return csv;
}

void Ensemble::giveMenu(QMenu *m, Display *d)
{
	if (ensembleCount() > 1)
	{
		QAction *a = m->addAction("cluster4x");
		connect(a, &QAction::triggered, [=]{ d->cluster(this); });
		a = m->addAction("Torsion heat");
		connect(a, &QAction::triggered, [=]{ torsionHeat(); });
		a = m->addAction("Contact heat");
		connect(a, &QAction::triggered, [=]{ contactHeat(); });
	}
}

void Ensemble::contactHeat()
{
	_composite->contactHeatForEnsemble(this);
}

void Ensemble::torsionHeat()
{
	_composite->torsionHeatForEnsemble(this);
}

void Ensemble::changeEnsembleName(Ensemble *e, std::string before,
                                  std::string after)
{
	_nameMap.erase(before);
	_nameMap[after] = e;
}

void Ensemble::defineEntity(Entity *entity)
{
	for (size_t i = 0; i < chainCount(); i++)
	{
		if (chain(i)->entity() != NULL)
		{
			continue;
		}

		std::string seq = chain(i)->sequence();
		std::string refseq = entity->sequence();

		int muts, dels;
		compare_sequences(seq, refseq, &muts, &dels);
		
		if (muts < entity->tolerance())
		{
			setEntityAsChain(entity, chain(i)->str());
		}
	}

}

void Ensemble::takeEntityDefinition(std::string contents)
{
	if (contents.length() == 0)
	{
		return;
	}

	std::vector<std::string> lines = split(contents, '=');
	if (lines.size() < 2)
	{
		return;
	}

	Entity *entity = _composite->entity(lines[0]);
	std::vector<std::string> chains = split(lines[1], ';');
	
	for (size_t i = 0; i < chains.size(); i++)
	{
		std::cout << entity->name() << " " << chains[i] << std::endl;
		setEntityAsChain(entity, chains[i]);
	}
}

Chain *Ensemble::chainObjectForEntity(Entity *ent, int i)
{
	if (ent == NULL) return NULL;
	std::string ch = chainForEntity(ent, i);
	return _str2Chain[ch];
}

std::string Ensemble::chainForEntity(Entity *entity, int i)
{
	if (entity == NULL || _entity2Chain.count(entity) == 0)
	{
		return "";
	}

	if (_entity2Chain[entity].size() <= i)
	{
		return "";
	}

	return _entity2Chain[entity][i];
}
