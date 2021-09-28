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

#ifndef __switch__Ramaplot__
#define __switch__Ramaplot__

#include <h3dsrc/SlipObject.h>

class Ramapoint;
class Ensemble;
class CAlpha;

class Ramaplot : public SlipObject
{
public:
	Ramaplot(CAlpha *a);
	~Ramaplot();

	void loadFromAtomForEnsemble(Ensemble *e);
	void bestContacts(Ensemble *e);
	void processMovedToPoint(vec3 pos);
protected:
	virtual void bindTextures();
	virtual void render(SlipGL *gl);
private:
	Ramapoint *_points;
	CAlpha *_alpha;
	std::vector<Ensemble *> _extra;

};

#endif
