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

#ifndef __switch__Ramapoint__
#define __switch__Ramapoint__

#include <h3dsrc/SlipObject.h>

class Ensemble;

class Ramapoint : public SlipObject
{
public:
	Ramapoint(vec3 centre);

	void addPoint(Ensemble *e, vec3 pos);
	void processMovedToPoint(vec3 pos);
	void tidyUp();
private:
	vec3 _centre;
	std::vector<Ensemble *> _ensembles;

};

#endif
