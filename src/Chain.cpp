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

#include "Chain.h"
#include "Entity.h"
#include "Composite.h"
#include "Ensemble.h"
#include "CAlpha.h"
#include <iostream>
#include <hcsrc/maths.h>
#include <libsrc/Crystal.h>
#include <libsrc/Polymer.h>
#include <libsrc/Atom.h>
#include <libsrc/Monomer.h>
#include <h3dsrc/Icosahedron.h>
#include <hcsrc/Blast.h>
#include "Ensemble_sh.h"

#define DOTS_PER_BEZIER 10

Chain::Chain(CrystalPtr crystal, std::string chain) 
: SlipObject(), Handleable()
{
	_globalOffset = 0;
	_visibilityDemand = true;
	_entity = NULL;
	_chain = chain;
	_crystal = crystal;
	_renderType = GL_LINES;
	_vString = Ensemble_vsh();
	_fString = Ensemble_fsh();
	_vPerAtom = 0;

	setName(crystal->getAbsolutePath() + " chain " + _chain);
}

void control_points(vec3 *a, vec3 b, vec3 c, vec3 *d)
{
	vec3 ca = vec3_subtract_vec3(c, *a);
	vec3 bd = vec3_subtract_vec3(b, *d);
	vec3_mult(&ca, 0.4);
	vec3_mult(&bd, 0.4);
	*a = b;
	*d = c;
	vec3_add_to_vec3(a, ca);
	vec3_add_to_vec3(d, bd);
}

vec3 bezier(vec3 p1, vec3 p2, vec3 p3, vec3 p4, double t)
{
	double c1 = (1 - t) * (1 - t) * (1 - t);
	double c2 = 3 * t * (1 - t) * (1 - t);
	double c3 = 3 * t * t * (1 - t);
	double c4 = t * t * t;

	vec3_mult(&p2, c1);
	vec3_mult(&p3, c4);
	vec3_mult(&p1, c2);
	vec3_mult(&p4, c3);
	
	vec3 add = vec3_add_vec3(p1, p2);
	vec3_add_to_vec3(&add, p3);
	vec3_add_to_vec3(&add, p4);
	
	return add;
}

void Chain::addCircle(vec3 centre, std::vector<vec3> &circle)
{
	for (size_t i = 0; i < circle.size(); i++)
	{
		vec3 pos = vec3_add_vec3(centre, circle[i]);
		addVertex(pos.x, pos.y, pos.z);
	}
}

void Chain::addCylinderIndices(size_t num)
{
	int begin = - num * 2;
	int half = num;

	for (size_t i = 0; i < num - 1; i++)
	{
		addIndex(begin + 0);
		addIndex(begin + half);
		addIndex(begin + 1);
		addIndex(begin + 1);
		addIndex(begin + half);
		addIndex(begin + half + 1);
		begin++;

	}

	int one = num;
	half -= num * 2 - 1;
	addIndex(begin + 0);
	addIndex(begin + half);
	addIndex(begin + one);
	addIndex(begin + half);
	addIndex(begin + one);
	addIndex(begin + 1);
}

void Chain::convertToCylinder()
{
	std::vector<Helen3D::Vertex> orig = _vertices;
	_vertices.clear();
	_indices.clear();
	bool starter = true;
	const int divisions = 5;
	vec3 prev_axis = make_vec3(0, 0, 1);
	vec3 xAxis = make_vec3(1, 0, 0);
	
	if (orig.size() == 0)
	{
		return;
	}

	for (size_t i = 0; i < orig.size() - 1; i++)
	{
		vec3 v1 = vec_from_pos(orig[i].pos);
		vec3 v2 = vec_from_pos(orig[i + 1].pos);
		
		vec3 axis = vec3_subtract_vec3(v2, v1);

		vec3_set_length(&axis, 1);
		vec3 ave = vec3_add_vec3(axis, prev_axis);
		vec3_set_length(&ave, 1.);
		std::vector<vec3> circle;

		double angle = (2 * M_PI) / (double)divisions;
		mat3x3 rot = mat3x3_unit_vec_rotation(ave, angle);
		vec3 cross = vec3_cross_vec3(axis, xAxis);
		vec3_set_length(&cross, 0.2);
		prev_axis = axis;
		
		if (!vec3_is_sane(v1) || !vec3_is_sane(v2))
		{
//			continue;
		}

		for (size_t i = 0; i < divisions; i++)
		{
			circle.push_back(cross);
			mat3x3_mult_vec(rot, &cross);
		}

		if (starter)
		{
			addCircle(v1, circle);
		}

		addCircle(v2, circle);
		
		addCylinderIndices(circle.size());

		
		starter = false;
	}
	
	_renderType = GL_TRIANGLES;
	calculateNormals();
}

void Chain::render(SlipGL *gl)
{
	if (!tryLockMutex())
	{
		return;
	}

	for (size_t i = 0; i < _balls.size(); i++)
	{
		_balls[i]->render(gl);
	}
	unlockMutex();

	SlipObject::render(gl);
}

void Chain::convertToBezier()
{
	std::vector<Helen3D::Vertex> vs = _vertices;
	std::vector<GLuint> is = _indices;
	_vertices.clear();
	_indices.clear();
	vec3 nanVec = make_vec3(NAN, NAN, NAN);
	
	if (is.size() == 0)
	{
		return;
	}

	for (int i = 0; i < (int)is.size() - 1; i += 2)
	{
		vec3 p2 = vec_from_pos(vs[is[i]].pos);
		vec3 p3 = vec_from_pos(vs[is[i + 1]].pos);
		
		bool sameBefore = (i <= 1);
		if (!sameBefore)
		{
			sameBefore |= (is[i - 1] != is[i]);
		}

		bool sameAfter = (i >= (int)is.size() - 2);
		
		if (!sameAfter)
		{
			sameAfter = (is[i + 2] != is[i + 1]);
		}

		if (is[i] < 0)
		{
			sameBefore = true;
		}
		
		if (is[i + 2] >= _vertices.size())
		{
			sameAfter = true;
		}
		
		vec3 p1 = p2;
		vec3 p4 = p3;
		
		if (!sameBefore)
		{
			p1 = vec_from_pos(vs[is[i] - 1].pos);
		}
		if (!sameAfter)
		{
			p4 = vec_from_pos(vs[is[i] + 1].pos);
		}
		
		control_points(&p1, p2, p3, &p4);
		
		for (int ti = 0; ti < 10; ti++)
		{
			double t = (double)ti / 10;

			if (sameBefore)// && ti == 0)
			{
				addVertex(nanVec, NULL);
			}

			_indices.push_back(_vertices.size());
			_indices.push_back(_vertices.size() + 1);

			vec3 p = bezier(p1, p2, p3, p4, t);
			
			if (p.x != p.x || p.y != p.y || p.z != p.z)
			{
				std::cout << "Nan!" << std::endl;
				std::cout << vec3_desc(p1) << std::endl;
				std::cout << vec3_desc(p2) << std::endl;
				std::cout << vec3_desc(p3) << std::endl;
				std::cout << vec3_desc(p4) << std::endl;
			}

			addVertex(p, NULL);
		}
		
		
		if (sameAfter)
		{
			_indices.pop_back();
			_indices.pop_back();
		}
	}
}

void Chain::addCAlpha(AtomPtr ca, vec3 point)
{
	bool lastOK = (_vertices.size() > 0);
	
	if (lastOK)
	{
		Helen3D::Vertex last = _vertices.back();
		lastOK = (last.pos[0] == last.pos[0]);
	}
	
	if (vec3_is_sane(point) && lastOK)
	{
		_indices.push_back(_vertices.size() - 1);
		_indices.push_back(_vertices.size());
	}

	Helen3D::Vertex v;
	memset(&v, 0, sizeof(Helen3D::Vertex));
	
	v.color[3] = 1;
	v.pos[0] = point.x;
	v.pos[1] = point.y;
	v.pos[2] = point.z;
	
	_vertices.push_back(v);
	
	if (ca)
	{
		_ca2Vertex[ca] = _vertices.size();
	}
	
	Icosahedron *ball = new Icosahedron();
	ball->resize(1.5);

	ball->triangulate();
	ball->setPosition(point);
	ball->setShadersLike(this);
	ball->setDisabled(true);

	_ball2Atom[ball] = ca;
	_atom2Ball[ca] = ball;
	_balls.push_back(ball);
}

CAlpha *Chain::whichAtom(double x, double y, double *z)
{
	AtomPtr atom = NULL;
	
	if (isDisabled())
	{
		return NULL;
	}
	
	Icosahedron *ball = NULL;
	for (size_t i = 0; i < _balls.size(); i++)
	{
		_balls[i]->setModel(_model);
		_balls[i]->setProj(_proj);
		bool covers = _balls[i]->intersectsPolygon(x, y, z);
		if (covers)
		{
			ball = _balls[i];
		}
	}
	
	if (ball != NULL)
	{
		atom = _ball2Atom[ball];
	}
	
	CAlpha *ca = _composite->getCAlpha(atom);
	if (ca == NULL)
	{
		return NULL;
	}
	
	std::cout << "Calpha map child has " << ca->children() << 
	" children from ";
	std::cout << _ensemble->title() << std::endl;
	
	return ca;
}

void Chain::getAlphas(Composite *c)
{
	for (size_t i = 0; i < _cas.size(); i++)
	{
		CAlpha *ca = c->getCAlpha(_cas[i]);
		if (ca)
		{
			_caMap[_cas[i]] = ca;
			ca->setChain(this);
		}
	}
}

void Chain::repopulate(mat3x3 r, vec3 t, vec3 loc)
{
	lockMutex();

	_indices.clear();
	_vertices.clear();
	_ca2Vertex.clear();
	_cas.clear();
	std::vector<Icosahedron *> tmp = _balls;
	_balls.clear();
	_ball2Atom.clear();
	_atom2Ball.clear();

	for (size_t i = 0; i < tmp.size(); i++)
	{
		delete tmp[i];
	}

	vec3 nanVec = make_vec3(NAN, NAN, NAN);

	for (size_t i = 0; i < _crystal->moleculeCount(); i++)
	{
		MoleculePtr m = _crystal->molecule(i);
		
		if (m->getChainID().substr(0, _chain.length()) != _chain)
		{
			continue;
		}
		
		if (!m->isPolymer())
		{
			continue;
		}
		
		PolymerPtr p = ToPolymerPtr(m);
		for (size_t j = p->monomerBegin(); j < p->monomerEnd(); j++)
		{
			AtomPtr a = p->getMonomer(j)->findAtom("CA");
			if (a)
			{
				vec3 pos = a->getAbsolutePosition();
				if (vec3_is_sane(t))
				{
					pos -= t;
					mat3x3_mult_vec(r, &pos);
					pos += loc;
				}

				addCAlpha(a, pos);
				_cas.push_back(a);
			}
		}

		addCAlpha(AtomPtr(), nanVec);
	}

	addCAlpha(AtomPtr(), nanVec);
	if (_cas.size()  == 0)
	{
		unlockMutex();
		return;
	}
	
	int start_size = vertexCount();
	
	convertToBezier();
	convertToCylinder();
	int end_size = vertexCount();

	double mult = end_size / (double)start_size;
	_vPerAtom = end_size / _cas.size();
	
	for (size_t i = 0; i < _cas.size(); i++)
	{
		size_t val = _ca2Vertex[_cas[i]];
		_ca2Vertex[_cas[i]] = val * mult;
	}
	
	unlockMutex();
}

void Chain::correlationToVertex(Helen3D::Vertex &v, double cc)
{
	vec3 colour;
	val_to_cluster4x_colour(cc, &colour.x, &colour.y, &colour.z);
	vec3_mult(&colour, 2 / 255.);
	pos_from_vec(v.color, colour);
	vec3 empty = empty_vec3();
	pos_from_vec(v.extra, empty);
}

void Chain::heatToVertex(Helen3D::Vertex &v, double heat)
{
	if (heat < 0) heat = 0;
	vec3 colour_start, colour_aim;
	if (heat >= -1 && heat < 0.5)
	{
		colour_start = make_vec3(0.4, 0.4, 0.4); // grey
		colour_aim = make_vec3(0.55, 0.45, 0.29); // straw
	}
	else if (heat >= 0.5 && heat < 1)
	{
		colour_start = make_vec3(0.55, 0.45, 0.29); // straw
		colour_aim = make_vec3(0.39, 0.46, 0.68); // blue
	}
	else if (heat >= 1 && heat < 2)
	{
		colour_start = make_vec3(0.39, 0.46, 0.68); // blue
		colour_aim = make_vec3(0.68, 0.16, 0.08); // cherry red
	}
	else if (heat >= 2 && heat < 3)
	{
		colour_start = make_vec3(0.68, 0.16, 0.08); // cherry red
		colour_aim = make_vec3(0.92, 0.55, 0.17); // orange
	}
	else if (heat >= 3)
	{
		colour_start = make_vec3(0.92, 0.55, 0.17); // orange
		colour_aim = make_vec3(0.89, 0.89, 0.16); // yellow
	}

	double mult = heat - 1;
	if (mult < 0) mult = 0;
	mult *= 3;
	heat = fmod(heat, 1);
	colour_aim -= colour_start;
	vec3_mult(&colour_aim, heat);
	colour_start += colour_aim;
	pos_from_vec(v.color, colour_start);
	vec3_mult(&colour_start, mult);
	pos_from_vec(v.extra, colour_start);
}

void Chain::correlationToCAlpha(Ensemble *ref, CAlpha *target)
{
	lockMutex();
	setColour(0.4, 0.4, 0.4);

	for (size_t i = 0; i < ref->cAlphaCount(); i++)
	{
		CAlpha *a = ref->cAlpha(i);
		CAlpha *ca = a->getAlphaForCrystal(_crystal);
		
		if (!ca)
		{
			continue;
		}

		double cc = CAlpha::scoreBetweenAlphas(target, a, 1);
		
		if (cc != cc)
		{
			continue;
		}

		size_t start = _ca2Vertex[ca->atom()];

		correlationToVertex(_vertices[start], cc);
		Helen3D::Vertex &v = _vertices[start];
		for (size_t j = start; j < start + _vPerAtom &&
		     j < vertexCount(); j++)
		{
			memcpy(_vertices[j].color, v.color, sizeof(GLfloat) * 4);
			memcpy(_vertices[j].extra, v.extra, sizeof(GLfloat) * 4);
		}
	}

	unlockMutex();
}

void Chain::recolour()
{
	setColour(0.4, 0.4, 0.4);

	std::vector<double> heats;
	for (size_t i = 0; i < _cas.size(); i++)
	{
		if (_caMap.count(_cas[i]))
		{
			double heat = _caMap[_cas[i]]->heat();
			heats.push_back(heat);
		}
	}
	
	double m = mean(heats);
	double stdev = standard_deviation(heats);

	for (size_t i = 0; i < _cas.size(); i++)
	{
		size_t start = _ca2Vertex[_cas[i]];
		
		double heat = 0;
		if (_caMap.count(_cas[i]))
		{
			heat = _caMap[_cas[i]]->heat();
		}
		
		heat -= m;
		heat /= stdev;
		heat += 1;
		if (heat != heat) heat = 0;
		
		heatToVertex(_vertices[start], heat);
		Helen3D::Vertex &v = _vertices[start];
		for (size_t j = start; j < start + _vPerAtom &&
		     j < vertexCount(); j++)
		{
			memcpy(_vertices[j].color, v.color, sizeof(GLfloat) * 4);
			memcpy(_vertices[j].extra, v.extra, sizeof(GLfloat) * 4);
		}
		
		Icosahedron *ball = _atom2Ball[_cas[i]];
		ball->setColour(v.color[0], v.color[1], v.color[2]);
		ball->setExtra(v.extra[0], v.extra[1], v.extra[2], 0);
	}
}

void Chain::deselectBalls()
{
	for (size_t i = 0; i < _balls.size(); i++)
	{
		_balls[i]->setDisabled(true);
	}
}

void Chain::selectBall(AtomPtr atom)
{
	CAlpha *ca = _composite->getCAlpha(atom);
	if (ca == NULL)
	{
		return;
	}
	CAlpha *myCa = ca->getAlphaForCrystal(_crystal);
	if (myCa == NULL)
	{
		return;
	}

	AtomPtr mine = myCa->atom();

	if (_atom2Ball.count(mine))
	{
		_atom2Ball[mine]->setDisabled(false);
	}
}

void Chain::setVisible(bool vis)
{
	if (vis && !_visibilityDemand)
	{
		vis = false;
	}

	setDisabled(!vis);

	if (!vis)
	{
		for (size_t i = 0; i < _balls.size(); i++)
		{
			_balls[i]->setDisabled(true);
		}
	}

	emit Handleable::changedVisible();
}

std::string Chain::title()
{
	std::string prep = str();
	if (_entity != NULL)
	{
		prep += " (" + _entity->name() + ")";
	}

	return prep;
}

void Chain::setEntity(Entity *entity)
{
	if (_entity != NULL)
	{
		return;
	}
	_entity = entity;
	_entity->addChain(this);
	emit Handleable::changedName();
}

int Chain::offsetFromChain(Chain *chain)
{
	int other_min, max;
	Ensemble *theirs = chain->ensemble();
	theirs->minMaxResidues(chain->str(), &other_min, &max);

	int my_min;
	_ensemble->minMaxResidues(str(), &my_min, &max);
	int diff = my_min - other_min;

	std::string myseq = _ensemble->generateSequence(this);
	std::string refseq = theirs->generateSequence(chain);

	int muts, dels;
	setup_alignment(&_a, "");
	setup_alignment(&_b, "");
	compare_sequences_and_alignments(refseq, myseq, &muts, &dels, _a, _b);
	_perResOffset.clear();

	std::map<int, size_t> freqs;
	for (size_t j = 0; j < myseq.size(); j++)
	{
		if (_b.mask[j] == MATCHED)
		{
			int dir = _b.map[j] - j;
			freqs[dir]++;
		}
	}

	std::map<int, size_t>::iterator it;
	size_t biggest = 0;
	int dir = 0;
	for (it = freqs.begin(); it != freqs.end(); it++)
	{
		if (it->second > biggest)
		{
			biggest = it->second;
			dir = it->first;
		}
	}
	
	for (size_t j = 0; j < myseq.size(); j++)
	{
		if (_a.mask[j] == MATCHED)
		{
			int offset = j - _a.map[j];
			chain->_perResOffset.push_back(diff - offset);
		}
		else
		{
			chain->_perResOffset.push_back(INT_MAX);
		}
	}
	
	chain->_globalOffset = my_min;

	return 0;
}

int Chain::offsetForRefResidue(int i)
{
	i -= _globalOffset;
	if (i < 0 || i >= _perResOffset.size()) return 0;
	return _perResOffset[i];
}

bool Chain::belongsToCollective(Collective *c)
{
	return (c->hasChain(this));
}

