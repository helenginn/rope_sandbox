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
#include "Handle.h"
#include "Handleable.h"

Handle::Handle(Handleable *object, Handle *parent) 
: QObject(NULL), QTreeWidgetItem(parent)
{
	_object = object;
	object->addHandle(this);
	
	getTitle();
	getVisible();

	connect(object, &Handleable::changedName, this, &Handle::getTitle);
	connect(object, &Handleable::changedVisible, this, &Handle::getVisible);
}

void Handle::getTitle()
{
	std::string title = _object->title();
	setText(0, QString::fromStdString(title));
}

void Handle::getVisible()
{
	Qt::CheckState ch = _object->isVisible() ? Qt::Checked : Qt::Unchecked;
	setCheckState(0, ch);

	for (size_t i = 0; i < childCount(); i++)
	{
		Handle *h = static_cast<Handle *>(child(i));
		h->getVisible();
	}
}

void Handle::updateVisible()
{
	bool checked = checkState(0) == Qt::Checked;
	_object->setVisible(checked);
	
	getVisible();
}

void Handle::giveMenu(QMenu *m, Display *d)
{
	_object->giveMenu(m, d);

}
