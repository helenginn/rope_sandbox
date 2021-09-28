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

#include "Complex.h"
#include "Chain.h"
#include "Ensemble.h"
#include "Entity.h"
#include <iostream>
#include <algorithm>
#include <libica/svdcmp.h>

void removeDisplacement(std::vector<vec3> &vecs)
{
	vec3 ave = empty_vec3();
	for (size_t i = 0; i < vecs.size(); i++)
	{
		ave += vecs[i];
	}
	vec3_mult(&ave, 1/(double)vecs.size());

	for (size_t i = 0; i < vecs.size(); i++)
	{
		vecs[i] -= ave;
	}
}

void applyRotation(mat3x3 &m, std::vector<vec3> &vecs)
{
	for (size_t i = 0; i < vecs.size(); i++)
	{
		mat3x3_mult_vec(m, &vecs[i]);
	}
}

mat3x3 bestRotation(std::vector<vec3> &as, std::vector<vec3> &bs)
{
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

	for (size_t n = 0; n < as.size(); n++)
	{
		for (size_t j = 0; j < dim; j++)
		{
			for (size_t i = 0; i < dim; i++)
			{
				vec3 a = as[n];
				vec3 b = bs[n];
				double add = *(&a.x + i) * *(&b.x + j);
				ptrs[i][j] += add;
			}
		}
	}

	int success = svdcmp((mat)ptrs, dim, dim, (vect) w, (mat) vPtrs);
	
	free(w);

	if (!success)
	{
		free(ptrs);
		free(vVals);
		free(vPtrs);
		return make_mat3x3();
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
	
	mat3x3 mrot = make_mat3x3();
	
	for (size_t i = 0; i < dim * dim; i++)
	{
		mrot.vals[i] = rotvals[i];
	}
	
	free(ptrs);
	free(vVals);
	free(vPtrs);
	free(rotvals);
	free(rot);

	return mrot;
}

Complex::Complex(std::string name) : Collective()
{
	_refEnsemble = NULL;
	_name = name;
}

void Complex::provideExample(Ensemble *e, std::string chains)
{
	for (size_t i = 0; i < chains.length(); i++)
	{
		std::string c;
		c = chains[i];
		
		Chain *chain = e->chain(c);
		
		if (chain == NULL)
		{
			std::cout << "Can't find chain " << c << " in " <<
			e->title() << std::endl;
			throw(-1);
		}
		
		if (chain->entity() == NULL)
		{
			std::cout << "Can't find entity definition for " << chain 
			<< std::endl;
		}

		Entity *ent = chain->entity();
		vec3 pos = e->centroidForChain(c);
		int min, max;
		e->minMaxResidues(c, &min, &max);
		
		_chains.push_back(chain);
		_defaultOffsets.push_back(min);
		_entities.push_back(ent);
		_positions.push_back(pos);
	}
	
	_refEnsemble = e;
	removeDisplacement(_positions);
	
	std::cout << "Found complex definition for " << _name;
	std::cout << " involving " << _entities.size() << " chains." << std::endl;
}

std::string Complex::findChainsInEnsemble(Ensemble *e)
{
	if (_results.count(e))
	{
		return _results[e];
	}

	std::string s = e->chainString();
	std::cout << "Starting " << e->title() << "(" << s << ")" << std::endl;
	
	for (int i = 0; i < s.length(); i++)
	{
		std::string ch; ch = s[i];
		Chain *chain = e->chain(ch);
		if (chain == NULL)
		{
			continue;
		}
		Entity *ent = chain->entity();
		if (ent == NULL)
		{
			s.erase(s.begin() + i);
			i--;
		}
	}
	
	if (s.length() < _entities.size())
	{
		_results[e] = "";
		return "";
	}

	std::cout << "Now " << s << std::endl;
	
	std::vector<std::string> options;

	std::sort(s.begin(), s.end());
	do 
	{
		std::string option;
		bool good = true;
		
		std::string proposed = s.substr(0, _entities.size());
		if (std::find(options.begin(), options.end(), proposed) != options.end())
		{
			continue;
		}

		for (size_t i = 0; i < s.length() && i < _entities.size(); i++)
		{
			std::string c; c = s[i];
			Chain *chain = e->chain(c);
			if (chain->entity() != _entities[i])
			{
				good = false;
				break;
			}

			option.push_back(chain->str()[0]);
		}
		
		if (good)
		{
			options.push_back(option);
		}
	} 
	while (std::next_permutation(s.begin(), s.end()));

	if (options.size() == 0)
	{
		_results[e] = "";
		return "";
	}

	double lowest = FLT_MAX;
	int index = -1;
	for (size_t i = 0; i < options.size(); i++)
	{
		std::vector<vec3> vecs;
		for (size_t j = 0; j < options[i].size(); j++)
		{
			std::string c;
			c = options[i][j];
			
			vec3 centroid = e->centroidForChain(c);
			vecs.push_back(centroid);
		}

		removeDisplacement(vecs);
		mat3x3 rot = bestRotation(vecs, _positions);
		applyRotation(rot, vecs);
		
		double disp = 0;
		for (size_t i = 0; i < vecs.size(); i++)
		{
			vecs[i] -= _positions[i];
			disp += vec3_sqlength(vecs[i]);
		}
		
		if (lowest > disp)
		{
			lowest = disp;
			index = i;
		}
		
		if (disp < 10)
		{
			break;
		}
	}
	
	if (index < 0)	
	{
		_results[e] = "";
		return "";
	}
	
	_results[e] = options[index];

	for (size_t i = 0; i < options[index].size(); i++)
	{
		std::string ch; ch = options[index][i];
		Chain *mine = e->chain(ch);
		Chain *reference = _chains[i];
		int offset = reference->offsetFromChain(mine);

		_offsets[mine] = offset;
		_members.push_back(mine);
	}
	
	std::cout << e->title() << " " << options[index] << std::endl;
	return options[index];
}

void Complex::setVisible(bool vis)
{
	for (size_t i = 0; i < _members.size(); i++)
	{
		_members[i]->setHide(!vis);
		_members[i]->setVisible(vis);
	}

	_visible = vis;
	emit changedVisible();
}

bool Complex::hasChain(Chain *ch)
{
	return std::find(_members.begin(), _members.end(), ch) != _members.end();
}
