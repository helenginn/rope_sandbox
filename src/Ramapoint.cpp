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

#include <iostream>
#include "Ramapoint.h"
#include "Ensemble.h"
#include "Chain.h"
#include "Ramaplot_sh.h"

Ramapoint::Ramapoint(vec3 centre) : SlipObject()
{
	_centre = centre;
	setNeedsExtra(true);
	_vString = Ramaplot_vsh();
	_fString = Ramapoint_fsh();
	_renderType = GL_POINTS;
}

void Ramapoint::addPoint(Ensemble *e, vec3 pos)
{
	addVertex(_centre);
	int c = vertexCount() - 1;
	_ensembles.push_back(e);

	_vertices[c].extra[0] = pos.x;
	_vertices[c].extra[1] = pos.y;
	_vertices[c].extra[2] = +0.01;
	_vertices[c].color[3] = 1;
	addIndex(-1);
}

void Ramapoint::processMovedToPoint(vec3 pos)
{
	vec3 c = _centre;
	double last = 1;
	vec3 model = mat4x4_mult_vec3(_model, c, &last);
	vec3 proj = mat4x4_mult_vec3(_proj, model, &last);
	vec3_mult(&proj, 1 / last);

	pos -= proj;
	pos.z = 0;
	vec3_mult(&pos, 3);
	
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		vec3 v = vec_from_pos(_vertices[i].extra);
		v -= pos;
		double l = vec3_length(v) * 2;
		double blue = exp(-l * l);
		l *= 2;
		double other = exp(-l * l);
		_vertices[i].color[2] = 5 * blue;
		_vertices[i].color[0] = 5 * other;
		_vertices[i].color[1] = 5 * other;
		if (l != l)
		{
			std::cout << l << std::endl;
		}
		
		for (size_t j = 0; j < _ensembles[i]->chainCount(); j++)
		{
			_ensembles[i]->chain(j)->setAlpha(other);
		}
	}
}

void Ramapoint::tidyUp()
{
	for (size_t i = 0; i < _ensembles.size(); i++)
	{
		for (size_t j = 0; j < _ensembles[i]->chainCount(); j++)
		{
			_ensembles[i]->chain(j)->setAlpha(1);
		}
	}
}
