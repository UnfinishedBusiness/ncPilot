#include "DXFParsePathAdaptor.h"
#include <dxf/spline/Bezier.h>
#include <EasyRender/logging/loguru.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

/**
 * Default constructor.
 */
DXFParsePathAdaptor::DXFParsePathAdaptor(void *easy_render_instance, void (*v)(PrimativeContainer *), void (*m)(PrimativeContainer*, nlohmann::json)) 
{
    this->current_layer = "default";
    this->filename = "";
    this->easy_render_instance = reinterpret_cast<EasyRender *>(easy_render_instance);
    this->view_callback = v;
    this->mouse_callback = m;
    this->smoothing = 0.003;
    this->scale = 1.0;
    this->chain_tolorance = 0.001;
}
void DXFParsePathAdaptor::SetScaleFactor(double scale)
{
    this->scale = scale;
}
void DXFParsePathAdaptor::SetSmoothing(double smoothing)
{
    this->smoothing = smoothing;
}
void DXFParsePathAdaptor::SetFilename(std::string f)
{
    this->filename = f;
}
void DXFParsePathAdaptor::SetChainTolorance(double chain_tolorance)
{
    this->chain_tolorance = chain_tolorance;
}
void DXFParsePathAdaptor::GetBoundingBox(nlohmann::json path_stack, double_point_t *bbox_min, double_point_t *bbox_max)
{
    bbox_max->x = -1000000;
    bbox_max->y = -1000000;
    bbox_min->x = 1000000;
    bbox_min->y = 1000000;
    for (nlohmann::json::iterator it = path_stack.begin(); it != path_stack.end(); ++it)
    {
        for (nlohmann::json::iterator path = (*it).begin(); path != (*it).end(); ++path)
        {
            if ((double)(*path)["x"] < bbox_min->x) bbox_min->x = (double)(*path)["x"];
            if ((double)(*path)["x"] > bbox_max->x) bbox_max->x = (double)(*path)["x"];
            if ((double)(*path)["y"] < bbox_min->y) bbox_min->y = (double)(*path)["y"];
            if ((double)(*path)["y"] > bbox_max->y) bbox_max->y = (double)(*path)["y"];
        }
    }
}
bool DXFParsePathAdaptor::CheckIfPointIsInsidePath(std::vector<double_point_t> path, double_point_t point)
{
    size_t polyCorners = path.size();
    size_t j = polyCorners-1;
    bool oddNodes=false;
    for (size_t i = 0; i < polyCorners; i++)
    {
        if (((path[i].y < point.y && path[j].y >= point.y)
        ||   (path[j].y < point.y && path[i].y >= point.y))
        &&  (path[i].x <= point.x || path[j].x <= point.x))
        {
            oddNodes^=(path[i].x + (point.y - path[i].y) / (path[j].y - path[i].y) * (path[j].x - path[i].x) < point.x);
        }
        j=i;
    }
    return oddNodes;
}
bool DXFParsePathAdaptor::CheckIfPathIsInsidePath(std::vector<double_point_t> path1, std::vector<double_point_t> path2)
{
    for (std::vector<double_point_t>::iterator it = path1.begin(); it != path1.end(); ++it)
    {
        if (this->CheckIfPointIsInsidePath(path2, (*it)))
        {
            return true;
        }
    }
    return false;
}
void DXFParsePathAdaptor::Finish()
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
    //LOG_F(INFO, "(DXFParsePathAdaptor::Finish) Interpolating %lu splines", this->splines.size());
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
                LOG_F(ERROR, "(DXFParsePathAdaptor::Finish) %s", e.what());
            }
        }
        delete curve;
    }
    //LOG_F(INFO, "(DXFParsePathAdaptor::Finish) Processing %lu polylines", this->polylines.size());
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
        double_point_t our_endpoint = {this->polylines[x].points.back().point.x * this->scale, this->polylines[x].points.back().point.y * this->scale};
        double_point_t our_startpoint = {this->polylines[x].points.front().point.x * this->scale, this->polylines[x].points.front().point.y * this->scale};
        for (nlohmann::json::iterator it = this->geometry_stack.begin(); it != this->geometry_stack.end(); ++it)
        {
            if ((*it)["type"] == "line")
            {
                double_point_t start_point = {(*it)["start"]["x"], (*it)["start"]["y"]};
                double_point_t end_point = {(*it)["end"]["x"], (*it)["end"]["y"]};
                if (g.distance(start_point, our_endpoint) < this->chain_tolorance)
                {
                    shared++;
                }
                if (g.distance(start_point, our_startpoint) < this->chain_tolorance)
                {
                    shared++;
                }
                if (g.distance(end_point, our_startpoint) < this->chain_tolorance)
                {
                    shared++;
                }
                if (g.distance(end_point, our_endpoint) < this->chain_tolorance)
                {
                    shared++;
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
    nlohmann::json chains = g.chainify(this->geometry_stack, this->chain_tolorance);
    double_point_t bb_min;
    double_point_t bb_max;
    this->GetBoundingBox(chains, &bb_min, &bb_max);
    for (size_t x = 0; x < chains.size(); x++)
    {
        bool is_inside_contour = false;
        std::vector<double_point_t> raw_chain;
        for (size_t i = 0; i < chains[x].size(); i++)
        {
            raw_chain.push_back({((double)chains[x][i]["x"] - bb_min.x) - ((bb_max.x - bb_min.x) / 2), ((double)chains[x][i]["y"] - bb_min.y) - ((bb_max.y - bb_min.y) / 2)});
        }
        std::vector<double_point_t> simplfied = g.simplify(raw_chain, this->smoothing);
        EasyPrimative::Path *p = this->easy_render_instance->PushPrimative(new EasyPrimative::Path(simplfied));
        p->properties->data["layer"] = this->current_layer;
        p->properties->data["filename"] = this->filename;
        p->properties->mouse_callback = this->mouse_callback;
        p->properties->matrix_callback = this->view_callback;
        p->is_closed = false;
    }
    for (std::vector<PrimativeContainer*>::iterator it = this->easy_render_instance->GetPrimativeStack()->begin(); it != this->easy_render_instance->GetPrimativeStack()->end(); ++it)
    {
        if ((*it)->properties->view == this->easy_render_instance->GetCurrentView())
        {
            if ((*it)->type == "path")
            {
                std::vector<double_point_t> this_path = (*it)->path->points;
                for (std::vector<PrimativeContainer*>::iterator it2 = this->easy_render_instance->GetPrimativeStack()->begin(); it2 != this->easy_render_instance->GetPrimativeStack()->end(); ++it2)
                {
                    if ((*it) != (*it2))
                    {
                        if ((*it2)->properties->view == this->easy_render_instance->GetCurrentView())
                        {
                            if ((*it2)->type == "path")
                            {
                                if (this->CheckIfPathIsInsidePath(this_path, (*it2)->path->points))
                                {
                                    this->easy_render_instance->SetColorByName((*it)->properties->color, "grey");
                                    (*it)->properties->data["is_inside_contour"] = true;
                                    break;
                                }
                                else
                                {
                                    this->easy_render_instance->SetColorByName((*it)->properties->color, "white");
                                    (*it)->properties->data["is_inside_contour"] = false;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
void DXFParsePathAdaptor::ExplodeArcToLines(double cx, double cy, double r, double start_angle, double end_angle, double num_segments)
{
    Geometry g;
    std::vector<Point> pointList;
	std::vector<Point> pointListOut; //List after simplification
    double_point_t start;
    double_point_t sweeper;
    double_point_t end;
    start.x = cx + (r * cosf((start_angle) * 3.1415926f / 180.0f));
	start.y = cy + (r * sinf((start_angle) * 3.1415926f / 180.0f));
    end.x = cx + (r * cosf((end_angle) * 3.1415926f / 180.0f));
	end.y = cy + (r * sinf((end_angle) * 3.1415926 / 180.0f));
    pointList.push_back(Point(start.x, start.y));
    double diff = MAX(start_angle, end_angle) - MIN(start_angle, end_angle);
	if (diff > 180) diff = 360 - diff;
	double angle_increment = diff / num_segments;
	double angle_pointer = start_angle + angle_increment;
    for (int i = 0; i < num_segments; i++)
	{
		sweeper.x = cx + (r * cosf((angle_pointer) * 3.1415926f / 180.0f));
		sweeper.y = cy + (r * sinf((angle_pointer) * 3.1415926f / 180.0f));
		angle_pointer += angle_increment;
        pointList.push_back(Point(sweeper.x, sweeper.y));
	}
    pointList.push_back(Point(end.x, end.y));
    g.RamerDouglasPeucker(pointList, 0.0005, pointListOut);
    for(size_t i=1; i< pointListOut.size(); i++)
	{
        this->addLine(DL_LineData((double)pointListOut[i-1].first, (double)pointListOut[i-1].second, 0, (double)pointListOut[i].first, (double)pointListOut[i].second, 0));
	}
}

void DXFParsePathAdaptor::addLayer(const DL_LayerData& data)
{
    current_layer = data.name;
}

void DXFParsePathAdaptor::addPoint(const DL_PointData& data)
{
    LOG_F(WARNING, "(DXFParsePathAdaptor::addPoint) No point handle!");
}

void DXFParsePathAdaptor::addLine(const DL_LineData& data)
{
    nlohmann::json g;
    g["type"] = "line";
    g["start"]["x"] = data.x1 * scale;
    g["start"]["y"] = data.y1 * scale;
    g["end"]["x"] = data.x2 * scale;
    g["end"]["y"] = data.y2 * scale;
    this->geometry_stack.push_back(g);
}
void DXFParsePathAdaptor::addXLine(const DL_XLineData& data)
{
    LOG_F(WARNING, "(DXFParsePathAdaptor::addXline) No Xline handle!");
}

void DXFParsePathAdaptor::addArc(const DL_ArcData& data)
{
    this->ExplodeArcToLines(data.cx, data.cy, data.radius, data.angle1, data.angle2, 100);
}

void DXFParsePathAdaptor::addCircle(const DL_CircleData& data)
{
    this->ExplodeArcToLines(data.cx, data.cy, data.radius, 0, 180, 100);
    this->ExplodeArcToLines(data.cx, data.cy, data.radius, 180, 360, 100);
}
void DXFParsePathAdaptor::addEllipse(const DL_EllipseData& data)
{
    LOG_F(WARNING, "(DXFParsePathAdaptor::addEllipse) No Ellipse handle!");
}

void DXFParsePathAdaptor::addPolyline(const DL_PolylineData& data)
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

void DXFParsePathAdaptor::addVertex(const DL_VertexData& data)
{
    polyline_vertex_t vertex;
    vertex.point.x = data.x;
    vertex.point.y = data.y;
    vertex.bulge = data.bulge;
    current_polyline.points.push_back(vertex);
}
void DXFParsePathAdaptor::addSpline(const DL_SplineData& data)
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
void DXFParsePathAdaptor::addControlPoint(const DL_ControlPointData& data)
{
    double_point_t p;
    p.x = data.x;
    p.y = data.y;
    current_spline.points.push_back(p);
}
void DXFParsePathAdaptor::addFitPoint(const DL_FitPointData& data)
{
    //LOG_F(WARNING, "(DXFParsePathAdaptor::addFitPoint) No FitPoint handle!");
}
void DXFParsePathAdaptor::addKnot(const DL_KnotData& data)
{
    //LOG_F(WARNING, "(DXFParsePathAdaptor::addKnot) No addKnot handle!");
}
void DXFParsePathAdaptor::addRay(const DL_RayData& data)
{
    //LOG_F(WARNING, "(DXFParsePathAdaptor::addRay) No addRay handle!");
}
