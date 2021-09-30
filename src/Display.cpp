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

#include "Display.h"
#include "Entity.h"
#include "StructureView.h"
#include "Composite.h"
#include "Handle.h"
#include "Ensemble.h"
#include <iostream>
#include <QMenu>
#include <QThread>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <libsrc/Crystal.h>
#include <c4xsrc/Screen.h>
#include <c4xsrc/Group.h>
#include <c4xsrc/ClusterList.h>

Display::Display(QWidget *parent) : QMainWindow(parent)
{
	_composite = NULL;
	_last = NULL;
	_screen = NULL;
	_treeTabs = NULL;
	_worker = new QThread();
	QHBoxLayout *hbox = new QHBoxLayout();
	QWidget *central = new QWidget(this);
	central->setLayout(hbox);
	
	QVBoxLayout *vbox = new QVBoxLayout();

	_treeTabs = new QTabWidget(NULL);
	vbox->addWidget(_treeTabs);
	_treeTabs->setMinimumSize(250, 0);
	_treeTabs->setMaximumSize(350, 2000);

	{
		_entiTree = new QTreeWidget(NULL);
		_entiTree->setHeaderLabel("Collections");
		_entiTree->setContextMenuPolicy(Qt::CustomContextMenu);
		_entiTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
		connect(_entiTree, &QTreeWidget::itemClicked, 
		        this, &Display::itemWasClicked);
		_treeTabs->addTab(_entiTree, "Entities");
	}

	{
		_viewTree = new QTreeWidget(NULL);
		_viewTree->setHeaderLabel("Models");
		_viewTree->setContextMenuPolicy(Qt::CustomContextMenu);
		_viewTree->setSelectionMode(QAbstractItemView::ExtendedSelection);

		connect(_viewTree, &QTreeWidget::customContextMenuRequested,
		        this, &Display::ensembleMenu);
		connect(_viewTree, &QTreeWidget::itemClicked, 
		        this, &Display::itemWasClicked);
		_treeTabs->addTab(_viewTree, "Models");
	}

	/*
	{
		_viewTree = new QTreeWidget(NULL);
		_viewTree->setMinimumSize(250, 0);
		_viewTree->setMaximumSize(350, 2000);
		_viewTree->setHeaderLabel("Models");
		_viewTree->setContextMenuPolicy(Qt::CustomContextMenu);
		_viewTree->setSelectionMode(QAbstractItemView::ExtendedSelection);

		connect(_viewTree, &QTreeWidget::customContextMenuRequested,
		        this, &Display::ensembleMenu);
		connect(_viewTree, &QTreeWidget::itemClicked, 
		        this, &Display::itemWasClicked);
		_treeTabs->addTab(_viewTree, "Models");
	}
	*/

	hbox->addLayout(vbox);

	hbox->addLayout(vbox);

	_structureView = new StructureView(NULL);
	_structureView->setMinimumSize(500, 500);
	_structureView->setDisplay(this);
	hbox->addWidget(_structureView);

	setCentralWidget(central);
	show();
}

void Display::itemWasClicked(QTreeWidgetItem *item, int column)
{
	Handle *h = dynamic_cast<Handle *>(item);
	if (h == NULL)
	{
		return;
	}

	if (column == 0)
	{
		h->updateVisible();
	}
}

Ensemble *Display::addCrystal(CrystalPtr c, Composite *comp, Ensemble *parent)
{
	c->refreshAnchors();
	Ensemble *e = new Ensemble(parent, c);
	e->setComposite(comp);
	e->setName(comp->nameForCrystal(c));
	e->setDisplay(this);

	if (!parent)
	{
		_structureView->addEnsemble(e);

		for (size_t i = 0; i < comp->entityCount(); i++)
		{
			Entity *entity = comp->entity(i);
			e->defineEntity(entity);
		}

		comp->addEnsemble(e);
		_ensembles.push_back(e);
		Handle *h = new Handle(e, NULL);
		h->moveToThread(_worker);
		e->chainsToHandle(h);
		_viewTree->addTopLevelItem(h);
	}
	else
	{
		e->repopulate();
		Handle *h = parent->firstHandle();
		h->moveToThread(_worker);
		Handle *sub = new Handle(e, h);
		h->addChild(sub);
		e->chainsToHandle(sub);
	}

	return e;
}

void Display::refreshEnsembles()
{
	for (size_t i = 0; i < _ensembles.size(); i++)
	{
		_ensembles[i]->repopulate();
	}
}

void Display::colourEnsembles()
{
	for (size_t i = 0; i < _ensembles.size(); i++)
	{
		_ensembles[i]->recolour();
	}
}

void Display::ensembleMenu(const QPoint &p)
{
	QMenu *m = new QMenu();
	QPoint pos = _viewTree->mapToGlobal(p);

	if (_viewTree->selectedItems().size() > 1)
	{
		return;
	}
	
	Handle *h = static_cast<Handle *>(_viewTree->currentItem());
	if (h)
	{
		h->giveMenu(m, this);
	}
	m->exec(pos);
}

void Display::addCollective(Collective *e)
{
	Handle *h = new Handle(e, NULL);
	h->moveToThread(_worker);
	_entiTree->addTopLevelItem(h);
}

void Display::clusterAtoms(Ensemble *e)
{
	if (_screen != NULL)
	{
		_screen->hide();
		_screen->deleteLater();
	}
	
	_composite->setReferenceTorsions(e);

	_screen = new Screen(NULL);
	_screen->setWindowTitle("cluster4x - rope");
	_last = e;
	
	e->prepareCluster4xPerAtom(_screen);
	_screen->addCSVSwitcher();

	_screen->show();
}

void Display::cluster(Ensemble *e)
{
	if (_screen != NULL)
	{
		_screen->hide();
		_screen->deleteLater();
	}
	
	_composite->setReferenceTorsions(e);

	_screen = new Screen(NULL);
	_screen->setWindowTitle("cluster4x - rope");
	_screen->setReturnJourney(this);
	_last = e;
	
	e->prepareCluster4x(_screen);
	_screen->addCSVSwitcher();

	_screen->show();
}

void Display::finished()
{
	ClusterList *list = _screen->getList();

	for (size_t i = 0; i < list->groupCount(); i++)
	{
		Group *g = list->group(i);
		
		if (!g->isMarked())
		{
			continue;
		}

		std::string def = "Split: " + _last->title();
		std::string custom = g->getCustomName();
		if (custom.length())
		{
			def = custom;
		}

		Ensemble *extra = addCrystal(_last->crystal(), _composite);
		extra->setName(def);
		Handle *exh = extra->firstHandle();

		for (size_t j = 0; j < g->mtzCount(); j++)
		{
			std::string name = g->getMetadata(j);
			Ensemble *e = _last->ensemble(name);
			if (e)
			{
				Handle *h = e->duplicateHandle();
				exh->addChild(h);
				extra->addEnsemble(e);
			}
		}
	}
	
	_screen->hide();
}

void Display::removeEnsemble(Ensemble *e)
{
	Handleable *he = static_cast<Handleable *>(e);
	for (size_t i = 1; i < _viewTree->topLevelItemCount(); i++)
	{
		Handle *h = static_cast<Handle *>(_viewTree->topLevelItem(i));
		if (h->object() == he)
		{
			_viewTree->takeTopLevelItem(i);
		}
	}
}

Ensemble *Display::activeEnsemble()
{
	QTreeWidgetItem *item = viewTree()->currentItem();
	Handle *h = static_cast<Handle *>(item);
	Ensemble *e = NULL;
	if (h != NULL)
	{
		e = dynamic_cast<Ensemble *>(h->object());
	}

	if (e == NULL && _ensembles.size())
	{
		e = _ensembles[0];
	}

	_composite->setReferenceTorsions(e);

	return e;
}
