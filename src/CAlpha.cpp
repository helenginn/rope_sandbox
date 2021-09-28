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

#include "CAlpha.h"
#include <libsrc/Twist.h>
#include <libsrc/Atom.h>
#include <libsrc/Anchor.h>
#include <libsrc/Crystal.h>
#include <libsrc/Bond.h>
#include <libsrc/Polymer.h>
#include <libsrc/Whack.h>
#include <hcsrc/FileReader.h>
#include <hcsrc/Canonical.h>

CAlpha::CAlpha(AtomPtr ca, CrystalPtr c)
{
	_chain = NULL;
	_ca = ca;
	_c = c;
	_heat = 0;
	_offset = empty_vec3();

	BondPtr parent = ToBondPtr(ca->getModel());
	_parent = parent;
	_phiBond = Bond::getRamachandranBond(parent, false);
	_psiBond = Bond::getRamachandranBond(parent, true);
	_phi = Bond::getTorsion(&*_phiBond);
	_psi = Bond::getTorsion(&*_psiBond);
//	_phi = _phiBond->getInitialTorsion();
//	_psi = _psiBond->getInitialTorsion();

	if (parent->hasWhack())
	{
		_whack = parent->getWhack();
		_kPhi = Whack::getWhack(&*_whack);
		_kPsi = Whack::getKick(&*_whack);
	}
}

void CAlpha::adjustFromReference(CAlpha *ca)
{
	double tphi = ca->phi();
	double tpsi = ca->psi();

	while (_phi < tphi - M_PI) _phi += 2 * M_PI;
	while (_phi >= tphi + M_PI) _phi -= 2 * M_PI;

	while (_psi < tpsi - M_PI) _psi += 2 * M_PI;
	while (_psi >= tpsi + M_PI) _psi -= 2 * M_PI;
}

void CAlpha::addToAverage(CAlpha *ca)
{
	_alphas.push_back(ca);
	_crystals.push_back(ca->crystal());
	ca->adjustFromReference(this);
	
	_crystal2Alpha[ca->crystal()] = ca;
}

void CAlpha::calculateAverages()
{
	_phi = 0;
	_psi = 0;
	double count = 0;
	
	for (size_t i = 0; i < _alphas.size(); i++)
	{
		_phi += _alphas[i]->phi();
		_psi += _alphas[i]->psi();
		count++;
	}

	_phi /= count;
	_psi /= count;
}

double CAlpha::scoreBetweenAlphas(CAlpha *a, CAlpha *b, int type)
{
	double dots = 0;
	std::vector<double> ms, ns;

	int msize = 0;
	int nsize = 0;
	int count = 0;

	for (size_t i = 0; i < a->_crystals.size(); i++)
	{
		CrystalPtr trial = a->_crystals[i];
		if (b->_crystal2Alpha.count(trial) == 0)
		{
			continue;
		}
		
		CAlpha *ca = a->getAlphaForCrystal(trial);
		CAlpha *cb = b->getAlphaForCrystal(trial);
		
		if (!ca || !cb)
		{
			continue;
		}
		
		double aphi = ca->deltaPhi(a);
		double apsi = ca->deltaPsi(a);
		vec3 offset = ca->offset();
		if (type == 0 || type == 1)
		{
			ms.push_back(aphi);
			ms.push_back(apsi);
			msize = 2;
		}
		else
		{
			ms.push_back(offset.x);
			ms.push_back(offset.y);
			ms.push_back(offset.z);
			msize = 3;
		}

		double bphi = cb->deltaPhi(b);
		double bpsi = cb->deltaPsi(b);
		offset = cb->offset();
		if (type == 0)
		{
			ns.push_back(bphi);
			ns.push_back(bpsi);
			nsize = 2;
		}
		else
		{
			ns.push_back(offset.x);
			ns.push_back(offset.y);
			ns.push_back(offset.z);
			nsize = 3;
		}

		count++;
	}
	
	if (count < 6)
	{
		return 0;
	}
	
	if (ns.size() == 0 || ms.size() == 0)
	{
		return 0;
	}
	
	Canonical c(nsize, msize);
	c.addVecs(ns, ms);
	
	try
	{
		c.run();
	}
	catch (int e)
	{
		return 0;
	}
	
	double cc = c.correlation();

	return (cc);
}

std::string CAlpha::displayName()
{
	std::string chain = atom()->getChainID() + "_";
	std::string num = i_to_str(atom()->getResidueNum());
	pad(num, '0', 4);

	return chain + num;
}

double CAlpha::fraction()
{
	double res = atom()->getResidueNum();
	PolymerPtr pol = ToPolymerPtr(atom()->getMolecule());
	double count = pol->monomerCount();
	return (res - pol->monomerBegin()) / count;
}

void CAlpha::contactHeat(AtomGroupPtr cas)
{
	_heat = 0;
	_myHeat = 0;

	if (!cas)
	{
		cas = AtomGroupPtr(new AtomGroup());
		cas->setTop(_c);
		for (size_t i = 0; i < _c->atomCount(); i++)
		{
			if (_c->atom(i)->isHeteroAtom() && !_c->atom(i)->isWater())
			{
				cas->addAtom(_c->atom(i));
			}
		}

		_close = cas;
	}
	else
	{
		_close = cas;
	}

	if (!children())
	{
		const double min = 5.;
		vec3 pos = _ca->getAbsolutePosition();

		AtomGroupPtr al;
		
		if (_close) 
		{
			al = _close;
		}
		else
		{
			al = cas->getAtomsInBox(pos, min, min, min, false);
			_close = al;
		}

		for (size_t i = 0; i < al->atomCount(); i++)
		{
			AtomPtr at = al->atom(i);

			if (at->getChainID()[0] == _ca->getChainID()[0])
			{
//				continue;
			}

			double dist = _ca->getDistanceFrom(&*at);
			double add = exp(-(dist * dist) / (2 * min * min));

			if (add != add || add < 1e-6)
			{
				continue;
			}

			_heat += add;
			_myHeat += add;
		}

		if (_heat != _heat || !std::isfinite(_heat))
		{
			_myHeat = 0;
			_heat = 0;
		}

		return;
	}
	
	for (size_t i = 0; i < _alphas.size(); i++)
	{
		_alphas[i]->contactHeat();
		_heat += _alphas[i]->heat();
	}

	_heat /= (double)alphaCount();

	double sq = 0;
	for (size_t i = 0; i < _alphas.size(); i++)
	{
		_alphas[i]->_myHeat -= _heat;
		double h = _alphas[i]->_myHeat;
		if (h > 0)
		{
			sq += h * h;
		}
	}
	
//	_heat = sqrt(sq) / (double)alphaCount();

	for (size_t i = 0; i < _alphas.size(); i++)
	{
		_alphas[i]->_heat = _heat;
	}
}

void CAlpha::torsionHeat(CAlpha *r)
{
	_heat = 0;

	if (!children())
	{
		_heat = sqrt(deltaPhi(r) * deltaPhi(r) + deltaPsi(r) * deltaPsi(r));
		_myHeat = _heat;
	}

	double sq = 0;
	for (size_t i = 0; i < _alphas.size(); i++)
	{
		_alphas[i]->torsionHeat(r);
		sq += _alphas[i]->heat();
	}
	
	sq = sqrt(sq);
	if (children())
	{
		_heat = sq;
		
		for (size_t i = 0; i < _alphas.size(); i++)
		{
			_alphas[i]->_heat = _heat;
		}
	}
}
