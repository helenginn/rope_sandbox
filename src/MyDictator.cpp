// 
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

#include "MyDictator.h"
#include <hcsrc/FileReader.h>
#include <QThread>
#include "Composite.h"
#include "Display.h"

MyDictator::MyDictator() : Dictator()
{
	_display = new Display();
	_composite = new Composite();
	_display->setComposite(_composite);
	std::cout << QThread::currentThread() << std::endl;
	prepareWorkForObject(_composite);
	_composite->setDisplay(_display);
}

void MyDictator::connectUp()
{
	prepareWorkForObject(_composite);
	connect(_composite, SIGNAL(resultReady()), this, SLOT(handleResult()),
	        Qt::UniqueConnection);
	startThread();
}

void MyDictator::handleResult()
{
	_w->quit();
	_w->wait();
	disconnect(this, &MyDictator::doThing, nullptr, nullptr);
	disconnect(this, &MyDictator::load1String, nullptr, nullptr);
	disconnect(this, &MyDictator::load2Strings, nullptr, nullptr);
	jobDone();
}

void commaToLine(std::string &last)
{
	for (size_t i = 0; i < last.length(); i++)
	{
		if (last[i] == ',')
		{
			last[i] = '\n';
		}
	}
}

bool MyDictator::processRequest(std::string first, std::string last)
{
	if (first == "load-model" || first == "load-models")
	{
		commaToLine(last);
		_composite->loadModels(last);
	}
	else if (first == "load-list")
	{
		std::vector<std::string> bits = split(last, ',');
		bits.resize(2);
		connectUp();
		connect(this, &MyDictator::load2Strings, 
		        _composite, &Composite::loadFromFile);
		emit load2Strings(QString::fromStdString(bits[0]),
		QString::fromStdString(bits[1]));
		return false;
	}
	else if (first == "average-torsions")
	{
		_composite->setReferenceTorsions();
	}
	else if (first == "define-entity")
	{
		std::vector<std::string> bits = split(last, ',');
		to_lower(bits[0]);
		_composite->defineEntity(bits[0], bits[1]);
	}
	else if (first == "select-entity")
	{
		to_lower(last);
		_composite->selectEntity(last);
	}
	else if (first == "chain")
	{
		_composite->setChains(last);
	}
	else if (first == "align")
	{
		to_lower(last);
		connectUp();
		connect(this, &MyDictator::load1String, _composite, &Composite::align);
		emit load1String(QString::fromStdString(last));
		return false;
	}
	else if (first == "show-non-entities")
	{
		_composite->hideNonEntities(false);
	}
	else if (first == "hide-non-entities")
	{
		_composite->hideNonEntities(true);
	}
	else if (first == "hide-entity")
	{
		_composite->hideEntity(last);
	}
	else if (first == "contact-heat")
	{
		connectUp();
		connect(this, &MyDictator::doThing,
		        _composite, &Composite::contactHeat);
		emit doThing();
		return false;
		_composite->contactHeat();
	}
	else if (first == "torsion-heat")
	{
		connectUp();
		connect(this, &MyDictator::doThing,
		        _composite, &Composite::torsionHeat);
		emit doThing();
		return false;
		_composite->torsionHeat();
	}
	else if (first == "heat-from-csv")
	{
		_composite->heatFromCSV(last);
	}
	else if (first == "structure-vectors")
	{
		connectUp();
		connect(this, &MyDictator::load1String, _composite, &Composite::vectors);
		emit load1String(QString::fromStdString(last));
		return false;
	}
	else if (first == "torsion-correlations")
	{
		_composite->findCorrelations(last);
	}
	else if (first == "write-pdb")
	{
		_composite->writePDB(last);
	}
	else if (first == "quit")
	{
		exit(0);
	}

	return true;
}

