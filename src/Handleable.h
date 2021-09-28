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

#ifndef __switch__Handleable__
#define __switch__Handleable__

#include <string>
#include <QObject>

class QMenu;
class Handle;
class Display;

class Handleable : public QObject
{
Q_OBJECT
public:
	Handleable();
	virtual ~Handleable() {};

	virtual std::string title() = 0;
	virtual void setTitle(std::string title) {};
	virtual bool isVisible() = 0;
	virtual void setVisible(bool vis) = 0;
	virtual void giveMenu(QMenu *m, Display *d) {};
	
	void addHandle(Handle *h)
	{
		_handles.push_back(h);
	}
	
	Handle *firstHandle()
	{
		if (_handles.size() == 0)
		{
			return NULL;
		}

		return _handles[0];
	}
	
	int handleCount()
	{
		return _handles.size();
	}
	
	Handle *handle(int i)
	{
		return _handles[i];
	}

signals:
	void changedName();
	void changedVisible();
	void updateVisible();
protected:
	void emitVisible();
	
	void changeName(std::string name);
private:

	std::vector<Handle *> _handles;
};

#endif
