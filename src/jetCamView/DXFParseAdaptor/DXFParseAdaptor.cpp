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
