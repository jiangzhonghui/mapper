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


#include "transformation.h"

#include <qmath.h>

#include "template.h"
#include "matrix.h"

// ### PassPoint ###

void PassPoint::save(QIODevice* file)
{
	file->write((const char*)&src_coords, sizeof(MapCoordF));
	file->write((const char*)&dest_coords, sizeof(MapCoordF));
	file->write((const char*)&calculated_coords, sizeof(MapCoordF));
	file->write((const char*)&error, sizeof(double));
}
void PassPoint::load(QIODevice* file, int version)
{
	if (version < 27)
	{
		MapCoordF src_coords_template;
		file->read((char*)&src_coords_template, sizeof(MapCoordF));
	}
	file->read((char*)&src_coords, sizeof(MapCoordF));
	file->read((char*)&dest_coords, sizeof(MapCoordF));
	file->read((char*)&calculated_coords, sizeof(MapCoordF));
	file->read((char*)&error, sizeof(double));
}

// ### PassPointList ###

bool PassPointList::estimateSimilarityTransformation(TemplateTransform* transform)
{
	int num_pass_points = (int)size();
	if (num_pass_points == 1)
	{
		PassPoint* point = &at(0);
		MapCoordF offset = point->dest_coords - point->src_coords;
		
		transform->template_x += qRound64(1000 * offset.getX());
		transform->template_y += qRound64(1000 * offset.getY());
		point->calculated_coords = point->dest_coords;
		point->error = 0;
	}
	else if (num_pass_points >= 2)
	{
		// Create linear equation system and solve using the pseuo inverse
		Matrix mat(2*num_pass_points, 4);
		Matrix values(2*num_pass_points, 1);
		for (int i = 0; i < num_pass_points; ++i)
		{
			PassPoint* point = &at(i);
			mat.set(2*i, 0, point->src_coords.getX());
			mat.set(2*i, 1, point->src_coords.getY());
			mat.set(2*i, 2, 1);
			mat.set(2*i, 3, 0);
			mat.set(2*i+1, 0, point->src_coords.getY());
			mat.set(2*i+1, 1, -point->src_coords.getX());
			mat.set(2*i+1, 2, 0);
			mat.set(2*i+1, 3, 1);
			
			values.set(2*i, 0, point->dest_coords.getX());
			values.set(2*i+1, 0, point->dest_coords.getY());
		}
		
		Matrix transposed;
		mat.transpose(transposed);
		
		Matrix mat_temp, mat_temp2, pseudo_inverse;
		transposed.multiply(mat, mat_temp);
		if (!mat_temp.invert(mat_temp2))
			return false;
		mat_temp2.multiply(transposed, pseudo_inverse);
		
		// Calculate transformation parameters
		Matrix output;
		pseudo_inverse.multiply(values, output);
		
		double move_x = output.get(2, 0);
		double move_y = output.get(3, 0);
		double rotation = qAtan2((-1) * output.get(1, 0), output.get(0, 0));
		double scale = output.get(0, 0) / qCos(rotation);
		
		// Calculate transformation matrix
		double cosr = cos(rotation);
		double sinr = sin(rotation);
		
		Matrix trans_change(3, 3);
		trans_change.set(0, 0, scale * cosr);
		trans_change.set(0, 1, scale * (-sinr));
		trans_change.set(1, 0, scale * sinr);
		trans_change.set(1, 1, scale * cosr);
		trans_change.set(0, 2, move_x);
		trans_change.set(1, 2, move_y);
		
		// Transform the original transformation parameters to get the new transformation
		transform->template_scale_x *= scale;
		transform->template_scale_y *= scale;
		transform->template_rotation -= rotation;
		qint64 temp_x = qRound64(1000.0 * (trans_change.get(0, 0) * (transform->template_x/1000.0) + trans_change.get(0, 1) * (transform->template_y/1000.0) + trans_change.get(0, 2)));
		transform->template_y = qRound64(1000.0 * (trans_change.get(1, 0) * (transform->template_x/1000.0) + trans_change.get(1, 1) * (transform->template_y/1000.0) + trans_change.get(1, 2)));
		transform->template_x = temp_x;
		
		// Transform the pass points and calculate error
		for (int i = 0; i < num_pass_points; ++i)
		{
			PassPoint* point = &at(i);
			
			point->calculated_coords = MapCoordF(trans_change.get(0, 0) * point->src_coords.getX() + trans_change.get(0, 1) * point->src_coords.getY() + trans_change.get(0, 2),
												 trans_change.get(1, 0) * point->src_coords.getX() + trans_change.get(1, 1) * point->src_coords.getY() + trans_change.get(1, 2));
			point->error = point->calculated_coords.lengthTo(point->dest_coords);
		}
	}
	
	return true;
}