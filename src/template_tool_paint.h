/*
 *    Copyright 2012 Thomas Schöps
 *    
 *    This file is part of OpenOrienteering.
 * 
 *    OpenOrienteering is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 * 
 *    OpenOrienteering is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 * 
 *    You should have received a copy of the GNU General Public License
 *    along with OpenOrienteering.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef _OPENORIENTEERING_TEMPLATE_TOOL_PAINT_H_
#define _OPENORIENTEERING_TEMPLATE_TOOL_PAINT_H_

#include <QDialog>

#include "tool.h"

QT_BEGIN_NAMESPACE
class QListWidgetItem;
class QDockWidget;
QT_END_NAMESPACE

class Map;
class Template;
class PaintOnTemplatePaletteWidget;

class PaintOnTemplateTool : public MapEditorTool
{
Q_OBJECT
public:
	PaintOnTemplateTool(MapEditorController* editor, QAction* tool_button, Template* temp);
    virtual ~PaintOnTemplateTool();
	
	virtual void init();
	virtual QCursor* getCursor() {return cursor;}
	
	virtual bool mousePressEvent(QMouseEvent* event, MapCoordF map_coord, MapWidget* widget);
	virtual bool mouseMoveEvent(QMouseEvent* event, MapCoordF map_coord, MapWidget* widget);
    virtual bool mouseReleaseEvent(QMouseEvent* event, MapCoordF map_coord, MapWidget* widget);
	
	virtual void draw(QPainter* painter, MapWidget* widget);
	
	static QCursor* cursor;
	
public slots:
    void templateDeleted(int pos, Template* temp);
    void colorSelected(QColor color);
	
private:
	bool dragging;
	bool erasing;
	QColor paint_color;
	QRectF map_bbox;
	std::vector<MapCoordF> coords;
	
	Template* temp;
	QDockWidget* dock_widget;
	PaintOnTemplatePaletteWidget* widget;
	
	static int erase_width;
};

class PaintOnTemplatePaletteWidget : public QWidget
{
Q_OBJECT
public:
	PaintOnTemplatePaletteWidget(bool close_on_selection);
	
signals:
	void colorSelected(QColor color);
	
protected:
    virtual void paintEvent(QPaintEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
	
private:
	int getNumFieldsX();
	int getNumFieldsY();
	QColor getFieldColor(int x, int y);
	
	bool close_on_selection;
};

class PaintOnTemplateSelectDialog : public QDialog
{
Q_OBJECT
public:
	PaintOnTemplateSelectDialog(Map* map, QWidget* parent);
	
	inline Template* getSelectedTemplate() const {return selection;}
	
protected slots:
	void currentTemplateChanged(QListWidgetItem* current, QListWidgetItem* previous);
	
private:
	Template* selection;
	QPushButton* draw_button;
};

#endif