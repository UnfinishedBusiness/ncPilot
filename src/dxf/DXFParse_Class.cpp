/*
 * @file DXFParse_Class.cpp
 */

/*****************************************************************************
**  $Id: DXFParse_Class.cpp 8865 2008-02-04 18:54:02Z andrew $
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

#include <dxf/DXFParse_Class.h>
#include <json/json.h>
#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
/**
 * Default constructor.
 */
DXFParse_Class::DXFParse_Class() {}

std::string current_layer  = "default";
/**
 * Sample implementation of the method which handles layers.
 */
void DXFParse_Class::addLayer(const DL_LayerData& data) {
    current_layer = data.name;
    //printf("LAYER: %s flags: %d\n", data.name.c_str(), data.flags);
    //printAttributes();
}

/**
 * Sample implementation of the method which handles point entities.
 */
void DXFParse_Class::addPoint(const DL_PointData& data) {
    /*printf("POINT    (%6.3f, %6.3f, %6.3f)\n",
           data.x, data.y, data.z);
    printAttributes();*/
}

/**
 * Sample implementation of the method which handles line entities.
 */
void DXFParse_Class::addLine(const DL_LineData& data) {
    nlohmann::json line;
    line["layer"] = current_layer;
    line["type"] = "line";
    line["start"]["x"] = (double)data.x1;
    line["start"]["y"] = (double)data.y1;
    line["start"]["z"] = (double)data.z1;
    line["end"]["x"] = (double)data.x2;
    line["end"]["y"] = (double)data.y2;
    line["end"]["z"] = (double)data.z2;
    dxfJSON.push_back(line);
    //printf("LINE     (%6.3f, %6.3f, %6.3f) (%6.3f, %6.3f, %6.3f)\n", data.x1, data.y1, data.z1, data.x2, data.y2, data.z2);
    //printAttributes();
}
void DXFParse_Class::addXLine(const DL_XLineData& data)
{
    //printf("Adding XLine!\n");
}
/**
 * Sample implementation of the method which handles arc entities.
 */
void DXFParse_Class::addArc(const DL_ArcData& data) {
    nlohmann::json j;
    j["layer"] = current_layer;
    j["type"] = "arc";
    j["center"]["x"] = (double)data.cx;
    j["center"]["y"] = (double)data.cy;
    j["center"]["z"] = (double)data.cz;
    j["start_angle"] = (double)data.angle1;
    j["end_angle"] = (double)data.angle2;
    j["radius"] = (double)data.radius;
    dxfJSON.push_back(j);
    /*printf("ARC      (%6.3f, %6.3f, %6.3f) %6.3f, %6.3f, %6.3f\n",
           data.cx, data.cy, data.cz,
           data.radius, data.angle1, data.angle2);
    printAttributes();*/
}

/**
 * Sample implementation of the method which handles circle entities.
 */
void DXFParse_Class::addCircle(const DL_CircleData& data) {
    /*printf("CIRCLE   (%6.3f, %6.3f, %6.3f) %6.3f\n",
           data.cx, data.cy, data.cz,
           data.radius);
    printAttributes();*/
    nlohmann::json j;
    j["layer"] = current_layer;
    j["type"] = "circle";
    j["center"]["x"] = (double)data.cx;
    j["center"]["y"] = (double)data.cy;
    j["center"]["z"] = (double)data.cz;
    j["radius"] = (double)data.radius;
    dxfJSON.push_back(j);
}
void DXFParse_Class::addEllipse(const DL_EllipseData& data)
{
    //printf("Add Ellipse!\n");
}

/**
 * Sample implementation of the method which handles polyline entities.
 */
void DXFParse_Class::addPolyline(const DL_PolylineData& data) {
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
void DXFParse_Class::addVertex(const DL_VertexData& data) {
    //printf("\tVERTEX   (%6.3f, %6.3f, %6.3f) %6.3f\n", data.x, data.y, data.z, data.bulge);
    //printAttributes();
    polyline_vertex_t vertex;
    vertex.point.x = data.x;
    vertex.point.y = data.y;
    vertex.bulge = data.bulge;
    current_polyline.points.push_back(vertex);
}
void DXFParse_Class::addSpline(const DL_SplineData& data)
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
void DXFParse_Class::addControlPoint(const DL_ControlPointData& data)
{
    //printf("\tAdd control point!\n");
    double_point_t p;
    p.x = data.x;
    p.y = data.y;
    current_spline.points.push_back(p);
}
void DXFParse_Class::addFitPoint(const DL_FitPointData& data)
{
    //printf("\tAdd fit point!\n");
}
void DXFParse_Class::addKnot(const DL_KnotData& data)
{
    //printf("\tAdd knot!\n");
}
void DXFParse_Class::addRay(const DL_RayData& data)
{
    //printf("\tAdd Ray!\n");
}
void DXFParse_Class::printAttributes() {
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
