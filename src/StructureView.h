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

#ifndef __breathalyser__structureview__
#define __breathalyser__structureview__

#include <QObject>
#include <h3dsrc/SlipGL.h>

class Ensemble;
class Display;
class Text;

class StructureView : public SlipGL
{
public:
	StructureView(QWidget *parent);
	void addEnsemble(Ensemble *e);
	
	void setDisplay(Display *d)
	{
		_display = d;
	}
	
	Ensemble *firstEnsemble()
	{
		return _ensemble;
	}
protected:
	void clickMouse(double x, double y);

	virtual void mouseReleaseEvent(QMouseEvent *e);
	virtual void initializeGL();
private:
	Ensemble *_ensemble;
	Display *_display;
	Text *_text;
	vec3 _centre;
	bool _centreSet;

};

#endif
