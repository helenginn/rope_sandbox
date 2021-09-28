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

#include <hcsrc/FileReader.h>
#include "Entity.h"
#include "Ensemble.h"
#include "Chain.h"

Entity::Entity(std::string name) : Collective()
{
	_name = name;
	_tolerance = 30;
	_visible = true;
	_refMinRes = 0;
}

void Entity::setVisible(bool vis)
{
	for (size_t i = 0; i < _chains.size(); i++)
	{
		_chains[i]->setHide(!vis);
		_chains[i]->setVisible(vis);
	}

	_visible = vis;
	emit changedVisible();
}

void Entity::setSequenceFromChain(Chain *c)
{
	_seq = c->sequence();
	_reference = c;
	_refEnsemble = c->ensemble();
	sortReference();
}

void Entity::sortReference()
{
	int max;
	Ensemble *mine = _reference->ensemble();
	mine->minMaxResidues(_reference->str(), &_refMinRes, &max);
}

std::string Entity::title()
{
	return _name + " (" + i_to_str(_chains.size()) + " chains)";
}

std::string Entity::findChainsInEnsemble(Ensemble *e)
{
	if (_results.count(e))
	{
		return _results[e];
	}

	for (size_t i = 0; i < e->chainCount(); i++)
	{
		if (e->chain(i)->entity() == this)
		{
			Chain *mine = e->chain(i);
			int offset = _reference->offsetFromChain(mine);
			_results[e] = mine->str();
			_offsets[e->chain(i)] = offset;
			return _results[e];
		}
	}

	_results[e] = "";
	return "";
}
