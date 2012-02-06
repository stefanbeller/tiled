/*
 * cellpropertiestool.cpp
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010 Stefan Beller <stefanbeller@googlemail.com>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "cellpropertiestool.h"

#include "map.h"
#include "mapdocument.h"
#include "mapscene.h"
#include "tilelayer.h"
#include "propertiesdialog.h"

#include <math.h>
#include <QVector>
#include <QMainWindow>

using namespace Tiled;
using namespace Tiled::Internal;

CellPropertiesTool::CellPropertiesTool(QObject *parent)
    : AbstractTileTool(tr("Cell Properties"),
                       QIcon(QLatin1String(":images/24x24/document-properties.png")),
                       QKeySequence(tr("P")),
                       parent)
{
}

CellPropertiesTool::~CellPropertiesTool()
{
}

void CellPropertiesTool::mouseReleased(QGraphicsSceneMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
	}
}

void CellPropertiesTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) 
    {
	    TileLayer *tileLayer = currentTileLayer();
	    Q_ASSERT(tileLayer);
	
	    const QPoint tilePos = tilePosition();

		Cell *cell = tileLayer->getCellAt(tilePos.x(),tilePos.y());
		
	    PropertiesDialog propertiesDialog(tr("Cell"),
	                                      cell,
	                                      mapDocument()->undoStack(),
	                                      0);
	    propertiesDialog.exec();
    }
    
}

void CellPropertiesTool::tilePositionChanged(const QPoint &)
{
}

void CellPropertiesTool::languageChanged()
{
    setName(tr("Properties Tool"));
    setShortcut(QKeySequence(tr("P")));
}
