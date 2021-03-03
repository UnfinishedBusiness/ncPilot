#include "DXFParseAdaptor.h"
#include <dxf/spline/Bezier.h>
#include <EasyRender/logging/loguru.h>

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
    //LOG_F(INFO, "(DXFParseAdaptor::Finish) Interpolating %lu splines", this->splines.size());
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
        //LOG_F(INFO, "Spline Node Count: %d", curve->node_count());
        if (curve->node_count() > 0)
        {
            for (int i = 0; i < curve->node_count(); i++)
            {
                pointList.push_back(Point(curve->node(i).x, curve->node(i).y));
            }
            try
            {
                g.RamerDouglasPeucker(pointList, 0.003, out);
                for (int i = 1; i < out.size(); i++)
                {
                    this->addLine(DL_LineData((double)out[i-1].first, (double)out[i-1].second, 0, (double)out[i].first, (double)out[i].second, 0));
                }
                if (this->splines[x].isClosed == true)
                {
                    this->addLine(DL_LineData((double)out[0].first, (double)out[0].second, 0, (double)out[curve->node_count()-1].first, (double)out[curve->node_count()-1].second, 0));
                }
            }
            catch(const std::exception& e)
            {
                LOG_F(ERROR, "(DXFParseAdaptor::Finish) %s", e.what());
            }
        }
        delete curve;
    }
    //LOG_F(INFO, "(DXFParseAdaptor::Finish) Processing %lu polylines", this->polylines.size());
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
                    if (g.distance((*it)->line->start, our_endpoint) < 0.0005)
                    {
                        shared++;
                    }
                    if (g.distance((*it)->line->start, our_startpoint) < 0.0005)
                    {
                        shared++;
                    }
                    if (g.distance((*it)->line->end, our_startpoint) < 0.0005)
                    {
                        shared++;
                    }
                    if (g.distance((*it)->line->end, our_endpoint) < 0.0005)
                    {
                        shared++;
                    }
                }
                else if ((*it)->type == "arc")
                {
                    double_point_t start_point = g.create_polar_line((*it)->arc->center, (*it)->arc->start_angle, (*it)->arc->radius).end;
                    double_point_t end_point = g.create_polar_line((*it)->arc->center, (*it)->arc->end_angle, (*it)->arc->radius).end;
                    if (g.distance(start_point, our_startpoint) < 0.0005)
                    {
                        shared++;
                    }
                    if (g.distance(start_point, our_endpoint) < 0.0005)
                    {
                        shared++;
                    }
                    if (g.distance(end_point, our_startpoint) < 0.0005)
                    {
                        shared++;
                    }
                    if (g.distance(end_point, our_endpoint) < 0.0005)
                    {
                        shared++;
                    }
                }
            }
        }
        if (shared == 2)
        {
            if (this->polylines[x].points.back().bulge == 0.0f)
            {
                this->addLine(DL_LineData((double)this->polylines[x].points.front().point.x, (double)this->polylines[x].points.front().point.y, 0, (double)this->polylines[x].points.back().point.x, (double)this->polylines[x].points.back().point.y, 0));
            }
            else
            {
                double_point_t bulgeStart;
                bulgeStart.x = this->polylines[x].points.back().point.x;
                bulgeStart.y = this->polylines[x].points.back().point.y;
				double_point_t bulgeEnd;
                bulgeEnd.x = this->polylines[x].points.front().point.x;
                bulgeEnd.y = this->polylines[x].points.front().point.y;
				double_point_t midpoint = g.midpoint(bulgeStart, bulgeEnd);
				double distance = g.distance(bulgeStart, midpoint);
				double sagitta = this->polylines[x].points.back().bulge * distance;

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
        }
    }
}

void DXFParseAdaptor::addLayer(const DL_LayerData& data)
{
    current_layer = data.name;
}

void DXFParseAdaptor::addPoint(const DL_PointData& data)
{
    LOG_F(WARNING, "(DXFParseAdaptor::addPoint) No point handle!");
}

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
    LOG_F(WARNING, "(DXFParseAdaptor::addXline) No Xline handle!");
}

void DXFParseAdaptor::addArc(const DL_ArcData& data)
{
    EasyPrimative::Arc *a = this->easy_render_instance->PushPrimative(new EasyPrimative::Arc({data.cx, data.cy, data.cz}, (float)data.radius, (float)data.angle1, (float)data.angle2));
    a->properties->data["layer"] = this->current_layer;
    a->properties->data["filename"] = this->filename;
    a->properties->mouse_callback = this->mouse_callback;
    a->properties->matrix_callback = this->view_callback;
}

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
    LOG_F(WARNING, "(DXFParseAdaptor::addEllipse) No Ellipse handle!");
}

void DXFParseAdaptor::addPolyline(const DL_PolylineData& data)
{
    if ((data.flags & (1<<0)))
    {
        current_polyline.isClosed = true;
    }
    else
    {
        current_polyline.isClosed = false;
    }
    if (current_polyline.points.size() > 0)
    {
        polylines.push_back(current_polyline); //Push last polyline to
        current_polyline.points.clear();
    }
}

void DXFParseAdaptor::addVertex(const DL_VertexData& data)
{
    polyline_vertex_t vertex;
    vertex.point.x = data.x;
    vertex.point.y = data.y;
    vertex.bulge = data.bulge;
    current_polyline.points.push_back(vertex);
}
void DXFParseAdaptor::addSpline(const DL_SplineData& data)
{
    if ((data.flags & (1<<0)))
    {
        current_spline.isClosed = true;
    }
    else
    {
        current_spline.isClosed = false;
    }
    if (current_spline.points.size() > 0)
    {
        splines.push_back(current_spline); //Push last spline
        current_spline.points.clear();
    }
}
void DXFParseAdaptor::addControlPoint(const DL_ControlPointData& data)
{
    double_point_t p;
    p.x = data.x;
    p.y = data.y;
    current_spline.points.push_back(p);
}
void DXFParseAdaptor::addFitPoint(const DL_FitPointData& data)
{
    //LOG_F(WARNING, "(DXFParseAdaptor::addFitPoint) No FitPoint handle!");
}
void DXFParseAdaptor::addKnot(const DL_KnotData& data)
{
    //LOG_F(WARNING, "(DXFParseAdaptor::addKnot) No addKnot handle!");
}
void DXFParseAdaptor::addRay(const DL_RayData& data)
{
    //LOG_F(WARNING, "(DXFParseAdaptor::addRay) No addRay handle!");
}
