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


#include "tools_t.h"

#include "../src/file_format.h"
#include "../src/georeferencing.h"
#include "../src/global.h"
#include "../src/map_color.h"
#include "../src/map_editor.h"
#include "../src/map_grid.h"
#include "../src/map_widget.h"
#include "../src/mapper_resource.h"
#include "../src/object.h"
#include "../src/symbol.h"
#include "../src/symbol_line.h"
#include "../src/template.h"
#include "../src/tool_edit_point.h"

#include "../src/gui/main_window.h"

/// Creates a test map and provides pointers to specific map elements.
/// NOTE: delete the map manually in case its ownership is not transferred to a MapEditorController or similar!
struct TestMap
{
	Map* map;
	
	LineSymbol* line_symbol;
	PathObject* line_object;
	
	TestMap();
	~TestMap();
};


/// Creates a test map editor and provides related pointers.
struct TestMapEditor
{
	MainWindow* window;
	MapEditorController* editor;
	MapWidget* map_widget;
	
	TestMapEditor(Map* map);
	~TestMapEditor();
	
	void simulateClick(QPoint pos);
	void simulateClick(QPointF pos);
	void simulateDrag(QPoint start_pos, QPoint end_pos);
	void simulateDrag(QPointF start_pos, QPointF end_pos);
};


// ### TestMap ###

TestMap::TestMap()
{
	MapCoord coord;
	
	map = new Map();
	
	MapColor* black = new MapColor();
	black->setCmyk(MapColorCmyk(0.0f, 0.0f, 0.0f, 1.0f));
	black->setOpacity(1.0f);
	black->setName("black");
	map->addColor(black, 0);
	
	line_symbol = new LineSymbol();
	line_symbol->setLineWidth(1);
	line_symbol->setColor(black);
	map->addSymbol(line_symbol, 0);
	
	line_object = new PathObject(line_symbol);
	line_object->addCoordinate(MapCoord(10, 10));
	coord = MapCoord(20, 10);
	coord.setCurveStart(true);
	line_object->addCoordinate(coord);
	line_object->addCoordinate(MapCoord(20, 20));
	line_object->addCoordinate(MapCoord(30, 20));
	line_object->addCoordinate(MapCoord(30, 10));
	map->addObject(line_object);
	
	// TODO: fill map with more content as needed
}

TestMap::~TestMap()
{
}


// ### TestMapEditor ###

TestMapEditor::TestMapEditor(Map* map)
{
	window = new MainWindow(true);
	editor = new MapEditorController(MapEditorController::MapEditor, map);
	window->setController(editor);
	map_widget = editor->getMainWidget();
}

TestMapEditor::~TestMapEditor()
{
	delete window;
}

void TestMapEditor::simulateClick(QPoint pos)
{
	QTest::mouseClick(map_widget, Qt::LeftButton, 0, pos);
}

void TestMapEditor::simulateClick(QPointF pos)
{
	simulateClick(pos.toPoint());
}

void TestMapEditor::simulateDrag(QPoint start_pos, QPoint end_pos)
{
	QTest::mousePress(map_widget, Qt::LeftButton, 0, start_pos);
	
	// NOTE: the implementation of QTest::mouseMove() does not seem to work (tries to set the real cursor position ...)
	//QTest::mouseMove(map_widget, end_pos);
	// Use manual workaround instead which sends an event directly:
	QMouseEvent event(QEvent::MouseMove, map_widget->mapToGlobal(end_pos), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
	QApplication::sendEvent(map_widget, &event);
	
	QTest::mouseRelease(map_widget, Qt::LeftButton, 0, end_pos);
}

void TestMapEditor::simulateDrag(QPointF start_pos, QPointF end_pos)
{
	simulateDrag(start_pos.toPoint(), end_pos.toPoint());
}


// ### TestTools ###

void ToolsTest::initTestCase()
{
	Q_INIT_RESOURCE(resources);
	doStaticInitializations();
}


void ToolsTest::editTool()
{
	// Initialization
	TestMap map;
	TestMapEditor editor(map.map);
	EditTool* tool = new EditPointTool(editor.editor, NULL, editor.editor->getSymbolWidget());	// TODO: Refactor EditTool: MapEditorController and SymbolWidget pointers could be unnecessary
	editor.editor->setTool(tool);
	
	// Move the first coordinate of the line object
	MapWidget* map_widget = editor.map_widget;
	PathObject* object = map.line_object;
	
	MapCoord& coord = object->getCoordinate(0);
	QPointF coord_pos = map_widget->mapToViewport(coord);
	
	QPointF target_coord_pos = coord_pos + QPointF(0, -50);
	//MapCoord target_coord = map_widget->viewportToMap(target_coord_pos.toPoint());
	
	// Click to select the object
	editor.simulateClick(coord_pos);
	
	// Drag the coordinate to the new position
	editor.simulateDrag(coord_pos, target_coord_pos);
	
	// Check position deviation
	QPointF difference = map_widget->mapToViewport(coord) - target_coord_pos;
	QVERIFY(qAbs(difference.x()) == 0);
	QVERIFY(qAbs(difference.y()) < 0.1);
	
	// Cleanup
	editor.editor->setTool(NULL);
}


QTEST_MAIN(ToolsTest)