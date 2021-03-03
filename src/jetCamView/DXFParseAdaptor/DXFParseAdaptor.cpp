/*
 * @file DXFParseAdaptor.cpp
 */

/*****************************************************************************
**  $Id: DXFParseAdaptor.cpp 8865 2008-02-04 18:54:02Z andrew $
**
**  This is part of the dxflib library
**  Copyright (C) 2001 Andrew Mustun
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU Library General Public License as
**  published by the Free Software Foundation.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU Library General Public License for more details.
**
**  You should have received a copy of the GNU Library General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
******************************************************************************/
#include "DXFParseAdaptor.h"
#include <dxf/spline/Bezier.h>

/**
 * Default constructor.
 */
DXFParseAdaptor::DXFParseAdaptor(void *easy_render_instance, void (*v)(PrimativeContainer *), void (*m)(PrimativeContainer*, nlohmann::json)) 
{
    this->current_layer = "default";
    this->filename = "";
    this->easy_render_instance = reinterpret_cast<EasyRender *>(easy_render_instance);
    this->view_callback = v;
    this->mouse_callback = m;
}

void DXFParseAdaptor::SetFilename(std::string f)
{
    this->filename = f;
}

void DXFParseAdaptor::Finish()
{
    Geometry g = Geometry();
    if (this->current_polyline.points.size() > 0)
    {
        this->polylines.push_back(this->current_polyline); //Push last polyline
        this->current_polyline.points.clear();
    }
    if (this->current_spline.points.size() > 0)
    {
        this->splines.push_back(this->current_spline); //Push last spline
        this->current_spline.points.clear();
    }
    for (int x = 0; x < this->splines.size(); x++)
    {
        std::vector<Point> pointList;
        std::vector<Point> out;
        Curve* curve = new Bezier();
	    curve->set_steps(100);
        for (int y = 0; y < this->splines[x].points.size(); y++)
        {
            curve->add_way_point(Vector(this->splines[x].points[y].x, this->splines[x].points[y].y, 0));
        }
        for (int i = 0; i < curve->node_count(); i++)
        {
            pointList.push_back(Point(curve->node(i).x, curve->node(i).y));
        }
        g.RamerDouglasPeucker(pointList, 0.003, out);
        for (int i = 1; i < out.size(); i++)
        {
            this->addLine(DL_LineData((double)out[i-1].first, (double)out[i-1].second, 0, (double)out[i].first, (double)out[i].second, 0));
        }
        if (this->splines[x].isClosed == true)
        {
            this->addLine(DL_LineData((double)out[0].first, (double)out[0].second, 0, (double)out[curve->node_count()-1].first, (double)out[curve->node_count()-1].second, 0));
        }
        delete curve;
    }
    for (int x = 0; x < this->polylines.size(); x++)
    {
        for (int y = 0; y < this->polylines[x].points.size()-1; y++)
        {
            if (this->polylines[x].points[y].bulge != 0)
            {
                double_point_t bulgeStart;
                bulgeStart.x = this->polylines[x].points[y].point.x;
                bulgeStart.y = this->polylines[x].points[y].point.y;
				double_point_t bulgeEnd;
                bulgeEnd.x = this->polylines[x].points[y + 1].point.x;
                bulgeEnd.y = this->polylines[x].points[y + 1].point.y;
				double_point_t midpoint = g.midpoint(bulgeStart, bulgeEnd);
				double distance = g.distance(bulgeStart, midpoint);
				double sagitta = this->polylines[x].points[y].bulge * distance;

                double_line_t bulgeLine = g.create_polar_line(midpoint, g.measure_polar_angle(bulgeStart, bulgeEnd) + 270, sagitta);
                double_point_t arc_center = g.three_point_circle_center(bulgeStart, bulgeLine.end, bulgeEnd);
                double arc_endAngle, arc_startAngle = 0;
                if (sagitta > 0)
                {
                    arc_endAngle = g.measure_polar_angle(arc_center, bulgeEnd);
                    arc_startAngle = g.measure_polar_angle(arc_center, bulgeStart);
                }
                else
                {
                    arc_endAngle = g.measure_polar_angle(arc_center, bulgeStart);
                    arc_startAngle = g.measure_polar_angle(arc_center, bulgeEnd);
                }
                this->addArc(DL_ArcData((double)arc_center.x, (double)arc_center.y, 0, g.distance(arc_center, bulgeStart), arc_startAngle, arc_endAngle));
            }
            else
            {
                this->addLine(DL_LineData((double)this->polylines[x].points[y].point.x, (double)this->polylines[x].points[y].point.y, 0, (double)this->polylines[x].points[y+1].point.x, (double)this->polylines[x].points[y+1].point.y, 0));
            }
            
        }
        int shared = 0; //Assume we are not shared
        double_point_t our_endpoint = this->polylines[x].points.back().point;
        double_point_t our_startpoint = this->polylines[x].points.front().point;
        for (std::vector<PrimativeContainer*>::iterator it = this->easy_render_instance->GetPrimativeStack()->begin(); it != this->easy_render_instance->GetPrimativeStack()->end(); ++it)
        {
            if ((*it)->properties->view == this->easy_render_instance->GetCurrentView())
            {
                if ((*it)->type == "line")
                {
                    if ((*it)->line->start.x == our_endpoint.x && (*it)->line->start.y == our_endpoint.y)
                    {
                        shared++;
                    }
                    if ((*it)->line->end.x == our_endpoint.x && (*it)->line->end.x == our_endpoint.y)
                    {
                        shared++;
                    }
                    if ((*it)->line->start.x == our_startpoint.x && (*it)->line->start.y == our_startpoint.y)
                    {
                        shared++;
                    }
                    if ((*it)->line->end.x == our_startpoint.x && (*it)->line->end.y == our_startpoint.y)
                    {
                        shared++;
                    }
                }
                else if ((*it)->type == "arc")
                {
                    double_point_t center;
                    center.x = (*it)->arc->center.x;
                    center.y = (*it)->arc->center.y;
                    double_point_t start_point = g.create_polar_line(center, (*it)->arc->start_angle, (*it)->arc->radius).end;
                    double_point_t end_point = g.create_polar_line(center, (*it)->arc->end_angle, (*it)->arc->radius).end;
                    if (start_point.x == our_endpoint.x && start_point.y == our_endpoint.y)
                    {
                        shared++;
                    }
                    if (end_point.x == our_endpoint.x && end_point.y == our_endpoint.y)
                    {
                        shared++;
                    }
                    if (start_point.x == our_startpoint.x && start_point.y == our_startpoint.y)
                    {
                        shared++;
                    }
                    if (end_point.x == our_startpoint.x && end_point.y == our_startpoint.y)
                    {
                        shared++;
                    }
                }
            }
        }
        if (shared == 2)
        {
            /*DL_LineData data = DL_LineData();
            data.x1 = (double)this->polylines[x].points.front().point.x;
            data.y1 = (double)this->polylines[x].points.front().point.y;
            data.z1 = 0;
            data.x2 = (double)this->polylines[x].points.back().point.x;
            data.y2 = (double)this->polylines[x].points.back().point.y;
            data.z2 = 0;
            this->addLine(data);*/
        }
    }
}

/**
 * Sample implementation of the method which handles layers.
 */
void DXFParseAdaptor::addLayer(const DL_LayerData& data) {
    current_layer = data.name;
    //printf("LAYER: %s flags: %d\n", data.name.c_str(), data.flags);
    //printAttributes();
}

/**
 * Sample implementation of the method which handles point entities.
 */
void DXFParseAdaptor::addPoint(const DL_PointData& data) {
    /*printf("POINT    (%6.3f, %6.3f, %6.3f)\n",
           data.x, data.y, data.z);
    printAttributes();*/
}

/**
 * Sample implementation of the method which handles line entities.
 */
void DXFParseAdaptor::addLine(const DL_LineData& data)
{
    EasyPrimative::Line *l = this->easy_render_instance->PushPrimative(new EasyPrimative::Line({data.x1, data.y1, data.z1}, {data.x2, data.y2, data.z2}));
    l->properties->data["layer"] = this->current_layer;
    l->properties->data["filename"] = this->filename;
    l->properties->mouse_callback = this->mouse_callback;
    l->properties->matrix_callback = this->view_callback;
}
void DXFParseAdaptor::addXLine(const DL_XLineData& data)
{
    //printf("Adding XLine!\n");
}
/**
 * Sample implementation of the method which handles arc entities.
 */
void DXFParseAdaptor::addArc(const DL_ArcData& data)
{
    EasyPrimative::Arc *a = this->easy_render_instance->PushPrimative(new EasyPrimative::Arc({data.cx, data.cy, data.cz}, (float)data.radius, (float)data.angle1, (float)data.angle2));
    a->properties->data["layer"] = this->current_layer;
    a->properties->data["filename"] = this->filename;
    a->properties->mouse_callback = this->mouse_callback;
    a->properties->matrix_callback = this->view_callback;
}

/**
 * Sample implementation of the method which handles circle entities.
 */
void DXFParseAdaptor::addCircle(const DL_CircleData& data)
{
   EasyPrimative::Circle *c = this->easy_render_instance->PushPrimative(new EasyPrimative::Circle({data.cx, data.cy, data.cz}, (float)data.radius));
    c->properties->data["layer"] = this->current_layer;
    c->properties->data["filename"] = this->filename;
    c->properties->mouse_callback = this->mouse_callback;
    c->properties->matrix_callback = this->view_callback;
}
void DXFParseAdaptor::addEllipse(const DL_EllipseData& data)
{
    //printf("Add Ellipse!\n");
}

/**
 * Sample implementation of the method which handles polyline entities.
 */
void DXFParseAdaptor::addPolyline(const DL_PolylineData& data) {
    //printf("POLYLINE \n");
    //printf("flags: %d\n", (int)data.flags);
    //printAttributes();
    if ((data.flags & (1<<0)))
    {
        //printf("\tisClosed = true\n");
        current_polyline.isClosed = true;
    }
    else
    {
        //printf("\tisClosed = false\n");
        current_polyline.isClosed = false;
    }
    if (current_polyline.points.size() > 0)
    {
        polylines.push_back(current_polyline); //Push last polyline to
        current_polyline.points.clear();
    }
}


/**
 * Sample implementation of the method which handles vertices.
 */
void DXFParseAdaptor::addVertex(const DL_VertexData& data) {
    //printf("\tVERTEX   (%6.3f, %6.3f, %6.3f) %6.3f\n", data.x, data.y, data.z, data.bulge);
    //printAttributes();
    polyline_vertex_t vertex;
    vertex.point.x = data.x;
    vertex.point.y = data.y;
    vertex.bulge = data.bulge;
    current_polyline.points.push_back(vertex);
}
void DXFParseAdaptor::addSpline(const DL_SplineData& data)
{
    //printf("Spline - %d, flags: %d\n", splines.size(), data.flags);
    /*for (int i = 31; i >= 0; i--)
    {
        std::cout << ((data.flags >> i) & 1);
    }
    std::cout << "\n";*/
    if ((data.flags & (1<<0)))
    {
        //printf("\tisClosed = true\n");
        current_spline.isClosed = true;
    }
    else
    {
        //printf("\tisClosed = false\n");
        current_spline.isClosed = false;
    }
    /*if (data.flags == 0)
    {
        current_spline.isClosed = true; //Inksape puts a 0 flag when it's supposed to be closed apparently
    }*/
    if (current_spline.points.size() > 0)
    {
        splines.push_back(current_spline); //Push last spline
        current_spline.points.clear();
    }
}
void DXFParseAdaptor::addControlPoint(const DL_ControlPointData& data)
{
    //printf("\tAdd control point!\n");
    double_point_t p;
    p.x = data.x;
    p.y = data.y;
    current_spline.points.push_back(p);
}
void DXFParseAdaptor::addFitPoint(const DL_FitPointData& data)
{
    //printf("\tAdd fit point!\n");
}
void DXFParseAdaptor::addKnot(const DL_KnotData& data)
{
    //printf("\tAdd knot!\n");
}
void DXFParseAdaptor::addRay(const DL_RayData& data)
{
    //printf("\tAdd Ray!\n");
}
void DXFParseAdaptor::printAttributes() {
    /*printf("  Attributes: Layer: %s, ", attributes.getLayer().c_str());
    printf(" Color: ");
    if (attributes.getColor()==256)	{
        printf("BYLAYER");
    } else if (attributes.getColor()==0) {
        printf("BYBLOCK");
    } else {
        printf("%d", attributes.getColor());
    }
    printf(" Width: ");
    if (attributes.getWidth()==-1) {
        printf("BYLAYER");
    } else if (attributes.getWidth()==-2) {
        printf("BYBLOCK");
    } else if (attributes.getWidth()==-3) {
        printf("DEFAULT");
    } else {
        printf("%d", attributes.getWidth());
    }*/
    //printf(" Type: %s\n", attributes.getLineType().c_str());
}
    
    

// EOF
