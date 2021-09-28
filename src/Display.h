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

#ifndef __switch__Display__
#define __switch__Display__

#include <QMainWindow>
#include <boost/shared_ptr.hpp>
#include <c4xsrc/Screen.h>

namespace Vagabond { class Crystal; }
typedef boost::shared_ptr<Vagabond::Crystal> CrystalPtr;

class Screen;
class Collective;
class Ensemble;
class Composite;
class QTreeWidget;
class StructureView;
class QTreeWidgetItem;

class Display : public QMainWindow, public C4XAcceptor
{
Q_OBJECT
public:
	Display(QWidget *parent = NULL);

	Ensemble *addCrystal(CrystalPtr c, Composite *comp, 
	                     Ensemble *parent = NULL);
	void refreshEnsembles();
	void colourEnsembles();
	void addCollective(Collective *e);
	
	void setComposite(Composite *c)
	{
		_composite = c;
	}
	
	QTreeWidget *viewTree()
	{
		return _viewTree;
	}
	
	StructureView *structureView()
	{
		return _structureView;
	}
	
	Ensemble *activeEnsemble();

	void cluster(Ensemble *e);
	void clusterAtoms(Ensemble *e);
	void removeEnsemble(Ensemble *e);
	virtual void finished();
protected slots:
	void itemWasClicked(QTreeWidgetItem *item, int column);
	void ensembleMenu(const QPoint &p);
private:
	StructureView *_structureView;
	std::vector<Ensemble *> _ensembles;
	Ensemble *_last;

	QThread *_worker;
	QTreeWidget *_viewTree;
	QTreeWidget *_entiTree;
	Screen *_screen;
	Composite *_composite;
};

#endif

