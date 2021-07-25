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

#include <iostream>
#include <QMenu>
#include <h3dsrc/Text.h>
#include <hcsrc/FileReader.h>
#include "StructureView.h"
#include "Ensemble.h"
#include "Display.h"

StructureView::StructureView(QWidget *parent) : SlipGL(parent, true)
{
	_centreSet = false;
	setBackground(1, 1, 1, 1);
	setZFar(2000.);
	setFocusPolicy(Qt::ClickFocus);
	_text = NULL;
	_ensemble = NULL;
	_r = 0.5;
	_g = 0.5;
	_b = 0.5;
}

void StructureView::initializeGL()
{
	SlipGL::initializeGL();
}

void StructureView::addEnsemble(Ensemble *e)
{
	if (e == NULL)
	{
		return;
	}

	e->repopulate();

	addObject(e, false);

	if (_ensemble == NULL)
	{
		_ensemble = e;
		e->setReference(true);
	}
	
	if (!_centreSet)
	{
		vec3 centre = e->averagePos();
		_centre = centre;
		focusOnPosition(centre, 150);
		_centreSet = true;
	}
}

void StructureView::clickMouse(double x, double y)
{
	if (_ensemble != NULL)
	{
	}
}

void StructureView::mouseReleaseEvent(QMouseEvent *e)
{
	if (!_moving && e->button() == Qt::LeftButton)
	{
		double x = e->x(); double y = e->y();
		convertCoords(&x, &y);
		clickMouse(x, y);
	}
	/*
	if (!_moving && e->button() == Qt::RightButton)
	{
		double x = e->x(); double y = e->y();
		QPoint p = mapToGlobal(QPoint(x, y));
	}
	*/

	SlipGL::mouseReleaseEvent(e);
}
