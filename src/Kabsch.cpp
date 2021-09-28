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

#include "Kabsch.h"
#include "Collective.h"
#include <libica/svdcmp.h>
#include <libsrc/Crystal.h>
#include <libsrc/Polymer.h>
#include <libsrc/Monomer.h>
#include <libsrc/Atom.h>

Kabsch::Kabsch(CrystalPtr ref, CrystalPtr move)
{
	_ref = ref;
	_move = move;
	_collective = NULL;
	_trans = empty_vec3();
	_mrot = make_mat3x3();
	_target = empty_vec3();
}

void Kabsch::run()
{
	std::string refChain, moveChain;
	if (_collective)
	{
		refChain = _collective->findChainsInCrystal(_ref);
		moveChain = _collective->findChainsInCrystal(_move);
	}
	vec3 refCentre = empty_vec3();
	vec3 moveCentre = empty_vec3();
	double count = 0;
	AtomList tmp = _move->findAtoms("CA");
	AtomGroupPtr moveCas = AtomGroupPtr(new AtomGroup());
	moveCas->addAtomsFrom(tmp);

	for (size_t i = 0; i < _ref->moleculeCount(); i++)
	{
		char id = _ref->molecule(i)->getChainID()[0];
		std::string::iterator it;
		it = std::find(refChain.begin(), refChain.end(), id);
		
		if (it == refChain.end() && refChain.length())
		{
			continue;
		}

		std::string rel;

		if (!_ref->molecule(i)->isPolymer())
		{
			continue;
		}
		
		PolymerPtr pol = ToPolymerPtr(_ref->molecule(i));
		
		for (size_t j = pol->monomerBegin() + 2; j < pol->monomerEnd() - 2; j++)
		{
			if (!pol->getMonomer(j))
			{
				continue;
			}

			AtomPtr a = pol->getMonomer(j)->findAtom("CA");
			if (!a)
			{
				continue;
			}

			int offset = 0;
			int resi = a->getResidueNum();

			if (refChain.length())
			{
				size_t pos = it - refChain.begin();
				rel = moveChain[pos];
				offset = _collective->offsetForCrystalChain(_move, rel, resi);
			}
			
			resi -= offset;
			
			AtomList list = moveCas->findAtoms("CA", resi, rel);

			if (list.size() == 0)
			{
				continue;
			}
			
			AtomPtr b = list[0];
			
			vec3 aAbs = a->getAbsolutePosition();
			vec3 bAbs = b->getAbsolutePosition();
			
			if (!vec3_is_sane(aAbs) || !vec3_is_sane(bAbs))
			{
				continue;
			}
			
			_caMap[a] = b;
			_caMap[b] = a;
			_cas.push_back(a);
			
			refCentre += aAbs;
			moveCentre += bAbs;
			count++;
		}
	}

	if (count == 0)
	{
		return;
	}

	vec3_mult(&refCentre, 1 / count);
	vec3_mult(&moveCentre, 1 / count);
	
	const int dim = 3;
	
	double *vals = (double *)calloc(dim * dim, sizeof(double));
	double **ptrs = (double **)malloc(sizeof(double *) * dim);
	double *w = (double *)malloc(sizeof(double) * dim);

	double *vVals = (double *)calloc(dim * dim, sizeof(double));
	double **vPtrs = (double **)malloc(sizeof(double *) * dim);
	
	for (size_t i = 0; i < dim; i++)
	{
		ptrs[i] = &vals[i * dim];
		vPtrs[i] = &vVals[i * dim];
		w[i] = 0;
	}

	for (size_t n = 0; n < _cas.size(); n++)
	{
		for (size_t j = 0; j < dim; j++)
		{
			for (size_t i = 0; i < dim; i++)
			{
				vec3 a = _cas[n]->getAbsolutePosition();
				vec3 b = _caMap[_cas[n]]->getAbsolutePosition();
				a -= refCentre;
				b -= moveCentre;
				double add = *(&a.x + i) * *(&b.x + j);
				ptrs[i][j] += add;
			}
		}
	}

	int success = svdcmp((mat)ptrs, dim, dim, (vect) w, (mat) vPtrs);
	
	free(w);

	if (!success)
	{
		free(vals);
		free(ptrs);
		free(vVals);
		free(vPtrs);
		return;
	}

	double *rotvals = (double *)calloc(dim * dim, sizeof(double));
	double **rot = (double **)malloc(sizeof(double *) * dim);
	
	for (size_t i = 0; i < dim; i++)
	{
		rot[i] = &rotvals[i * dim];
	}
	
	for (size_t j = 0; j < dim; j++)
	{
		for (size_t i = 0; i < dim; i++)
		{
			for (size_t n = 0; n < dim; n++)
			{
				rot[i][j] += vPtrs[i][n] * ptrs[j][n];
			}
		}
	}
	
	_mrot = make_mat3x3();
	_trans = moveCentre;
	_target = refCentre;
	
	for (size_t i = 0; i < dim * dim; i++)
	{
		_mrot.vals[i] = rotvals[i];
	}
	
	free(vals);
	free(ptrs);
	free(vVals);
	free(vPtrs);
	free(rotvals);
	free(rot);
}

vec3 Kabsch::getDifference(AtomPtr a)
{
	if (_caMap.count(a) == 0)
	{
		return empty_vec3();
	}

	AtomPtr ref = _caMap[a];
	vec3 pos = ref->getAbsolutePosition();
	pos -= _target;
	mat3x3_mult_vec(_mrot, &pos);
	pos += _trans;
	pos -= a->getAbsolutePosition();

	return pos;
}
