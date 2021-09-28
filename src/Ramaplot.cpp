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
#include "Chain.h"
#include <hcsrc/maths.h>
#include "Ensemble.h"
#include <libsrc/Atom.h>
#include "Ramaplot.h"
#include "Ramapoint.h"
#include "Ramaplot_sh.h"
#include "paths.h"
#include <QImage>
#include <iostream>

Ramaplot::Ramaplot(CAlpha *a) : SlipObject()
{
	vec3 pos = a->atom()->getAbsolutePosition();

	_textured = true;
	setNeedsExtra(true);
	addVertex(pos);
	addVertex(pos);
	addVertex(pos);
	addVertex(pos);

	_vertices[0].extra[0] = -1;
	_vertices[0].extra[1] = -1;
	_vertices[1].extra[0] = -1;
	_vertices[1].extra[1] = +1;
	_vertices[2].extra[0] = +1;
	_vertices[2].extra[1] = -1;
	_vertices[3].extra[0] = +1;
	_vertices[3].extra[1] = +1;

	_vertices[0].tex[0] = 1;
	_vertices[0].tex[1] = 1;
	_vertices[1].tex[0] = 1;
	_vertices[1].tex[1] = 0;
	_vertices[2].tex[0] = 0;
	_vertices[2].tex[1] = 1;
	_vertices[3].tex[0] = 0;
	_vertices[3].tex[1] = 0;
	
	addIndices(0, 1, 2);
	addIndices(1, 2, 3);
	
	_vString = Ramaplot_vsh();
	_fString = Ramaplot_fsh();
	_points = new Ramapoint(pos);
	_alpha = a;
}

void Ramaplot::loadFromAtomForEnsemble(Ensemble *e)
{
	std::vector<double> xs, ys;
	std::vector<Ensemble *> es;

	for (size_t i = 0; i < e->ensembleCount(); i++)
	{
		Ensemble *minor = e->ensemble(i);
		CrystalPtr cryst = minor->crystal();
		CAlpha *next = _alpha->getAlphaForCrystal(cryst);
		if (next == NULL)
		{
			for (size_t j = 0; j < minor->chainCount(); j++)
			{
				minor->chain(j)->setAlpha(0);
			}
			_extra.push_back(minor);

			continue;
		}
		double x = next->deltaPhi(_alpha);
		double y = next->deltaPsi(_alpha);
		
		es.push_back(minor);
		xs.push_back(x);
		ys.push_back(y);
	}
	
	std::cout << es.size() << " accepted and " << _extra.size() << 
	" rejected." << std::endl;

	double xstdev = standard_deviation(xs);
	double ystdev = standard_deviation(ys);
	double xmean = mean(xs);
	double ymean = mean(ys);
	
	for (size_t i = 0; i < xs.size(); i++)
	{
		vec3 point = empty_vec3();
		point.x = (xs[i] - xmean) / (xstdev * 3);
		point.y = (ys[i] - ymean) / (ystdev * 3);
		_points->addPoint(es[i], point);
	}
	
	std::cout << "Alpha chosen has " << _alpha->alphaCount() << 
	" children" << std::endl;
	
	for (size_t i = 0; i < es.size(); i++)
	{
		for (size_t j = 0; j < es[i]->chainCount(); j++)
		{
			es[i]->chain(j)->correlationToCAlpha(e, _alpha);
		}
	}
	
//	bestContacts(e);
}

void Ramaplot::bestContacts(Ensemble *e)
{
	for (size_t i = 0; i < e->cAlphaCount(); i++)
	{
		CAlpha *other = e->cAlpha(i);
		double score = CAlpha::scoreBetweenAlphas(_alpha, other, true);
		if (score > 0.8)
		{
			std::cout << other->atom()->shortDesc() << ", " << std::endl;
		}
	}
}

void Ramaplot::bindTextures()
{
	std::string path = DATA_PREFIX;
	path += "/images/phi_psi_plot.png";
	QImage im(QString::fromStdString(path));
	
	bindOneTexture(&im, true);
}

void Ramaplot::render(SlipGL *gl)
{
	_points->render(gl);
	SlipObject::render(gl);
	_points->setModel(_model);
	_points->setProj(_proj);
}

Ramaplot::~Ramaplot()
{
	_points->tidyUp();
	for (size_t i = 0; i < _extra.size(); i++)
	{
		for (size_t j = 0; j < _extra[i]->chainCount(); j++)
		{
			_extra[i]->chain(j)->setAlpha(1);
		}
	}
	delete _points;
}

void Ramaplot::processMovedToPoint(vec3 pos)
{
	_points->processMovedToPoint(pos);
}

