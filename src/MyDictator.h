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

#ifndef __switch__MyDictator__
#define __switch__MyDictator__

#include <h3dsrc/Dictator.h>

class Composite;
class Display;

class MyDictator : public Dictator
{
Q_OBJECT
public:
	MyDictator();

	virtual void help() {};
	
	virtual bool processRequest(std::string first, std::string last);
signals:
	void load1String(QString filename);
	void load2Strings(QString filename, QString handle);
	void doThing();
private slots:
	void handleResult();
private:
	void connectUp();
	Composite *_composite;
	Display *_display;
};

#endif
