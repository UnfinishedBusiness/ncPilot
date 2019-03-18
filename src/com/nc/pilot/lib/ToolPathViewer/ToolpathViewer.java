package com.nc.pilot.lib.ToolPathViewer;

import com.nc.pilot.config.JetToolpathCutChartData;
import com.nc.pilot.lib.GlobalData;
import com.vividsolutions.jts.geom.Coordinate;
import com.vividsolutions.jts.geom.Geometry;
import com.vividsolutions.jts.geom.GeometryFactory;
import com.vividsolutions.jts.operation.buffer.BufferOp;
import com.vividsolutions.jts.operation.buffer.BufferParameters;
import org.kabeja.dxf.*;
import org.kabeja.parser.DXFParser;
import org.kabeja.parser.ParseException;
import org.kabeja.parser.Parser;
import org.kabeja.parser.ParserBuilder;

import java.awt.*;
import java.awt.geom.Line2D;
import java.io.IOException;
import java.util.*;

/**
 * Created by travis on 2/1/19.
 */

public class ToolpathViewer {

    private float[] job_stock_size = new float[] {48, 48};

    private Graphics2D g2d;
    public ArrayList<ViewerPart> ViewerPartStack = new ArrayList();
    private boolean isMousePressed = false;
    private float[] mouseLastDragPosition;

    // constructor
    public ToolpathViewer() {

    }
    public float getAngle(float[] start_point, float[] end_point) {
        float angle = (float) Math.toDegrees(Math.atan2(start_point[1] - end_point[1], start_point[0] - end_point[0]));

        angle += 180;
        if(angle >= 360){
            angle -= 360;
        }
        if(angle < 0){
            angle += 360;
        }

        return angle;
    }
    public boolean inTolerance(float a, float b, float t)
    {
        float diff;
        if (a > b)
        {
            diff = a - b;
        }
        else
        {
            diff = b - a;
        }
        //printf("(geoInTolerance) Difference: %.6f, Plus: %.6f, Minus: %.6f\n", diff, fabs(t), -fabs(t));
        if (diff <= Math.abs(t) && diff >= -Math.abs(t))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    public float[] getMidpoint(float[] start_point, float[] end_point) {
        return new float[] {(end_point[0] + start_point[0])/2,(end_point[1] + start_point[1])/2};
    }
    public float[] rotatePoint(float[] pivot, float[] rotated_point, float angle)
    {
        float s = (float)Math.sin(angle*Math.PI/180);
        float c = (float)Math.cos(angle*Math.PI/180);

        // translate point back to origin:
        rotated_point[0] -= pivot[0];
        rotated_point[1] -= pivot[1];

        // rotate point
        float xnew = (rotated_point[0] * c - rotated_point[1] * s);
        float ynew = (rotated_point[0] * s + rotated_point[1] * c);

        // translate point back:
        rotated_point[0] = xnew + pivot[0];
        rotated_point[1] = ynew + pivot[1];
        return new float[] {rotated_point[0], rotated_point[1]};
    }
    public float[] getPolarLineEndpoint(float[] start_point, float length, float angle)
    {
        float[] end_point = new float[] {start_point[0] + length, start_point[1]};
        return rotatePoint(start_point, end_point, angle);
    }
    // setter
    public void setJobMaterial(float width, float height)
    {
        job_stock_size[0] = width;
        job_stock_size[1] = height;
    }
    public float getAngularDifference(float beginAngle, float endAngle, int direction)
    {
        //Direction is positive 1 for inc plus or -1 for inc negative
        float difference = 0;
        if (direction > 0) //Inc +
        {
            if (beginAngle > endAngle)
            {
                difference = 360 - (beginAngle - endAngle);
            }
            else
            {
                difference = (endAngle - beginAngle);
            }
        }
        else //Inc -
        {
            if (beginAngle < endAngle)
            {
                difference = 360 - (endAngle - beginAngle);
            }
            else
            {
                difference = (beginAngle - endAngle);
            }
        }
        return difference;

    }
    public void RenderLine(float[] start, float end[])
    {
        g2d.draw(new Line2D.Float(((start[0] + GlobalData.work_offset[0]) * GlobalData.ViewerZoom) + GlobalData.ViewerPan[0], (((start[1] + GlobalData.work_offset[1]) * GlobalData.ViewerZoom) * -1) + GlobalData.ViewerPan[1], ((end[0] + GlobalData.work_offset[0]) * GlobalData.ViewerZoom) + GlobalData.ViewerPan[0], (((end[1] + GlobalData.work_offset[1]) * GlobalData.ViewerZoom) * -1) + GlobalData.ViewerPan[1]));
    }
    public void RenderArc(float[] start, float[] end, float[] center, float radius, String direction)
    {
        float start_angle = getAngle(center, start);
        float end_angle = getAngle(center, end);
        float number_of_segments = 200 * radius; //Scale number of segments by radius
        float angle_inc;
        float[] last_point = start;
        //System.out.println("start_angle: " + start_angle + " end_angle: " + end_angle);
        if (start_angle == end_angle) //We are a circle
        {
            float angularDifference = 360;
            angle_inc = angularDifference / number_of_segments;
            for (float x = 0; x < angularDifference; x += angle_inc)
            {
                if (direction == "CW")
                {
                    start_angle -= angle_inc;
                }
                else
                {
                    start_angle += angle_inc;
                }
                float [] new_point = getPolarLineEndpoint(center, radius, start_angle);
                RenderLine(last_point, new_point);
                last_point = new_point;
            }
        }
        else
        {
            if (direction == "CW")
            {
                float angularDifference = getAngularDifference(start_angle, end_angle, -1);
                angle_inc = angularDifference / number_of_segments;
                for (float x = 0; x < angularDifference - angle_inc; x += angle_inc)
                {
                    start_angle -= angle_inc;
                    float [] new_point = getPolarLineEndpoint(center, radius, start_angle);
                    RenderLine(last_point, new_point);
                    last_point = new_point;
                }
            }
            else
            {
                float angularDifference = getAngularDifference(start_angle, end_angle, 1);
                angle_inc = angularDifference / number_of_segments;
                for (float x = 0; x < angularDifference - angle_inc; x += angle_inc)
                {
                    start_angle += angle_inc;
                    float [] new_point = getPolarLineEndpoint(center, radius, start_angle);
                    RenderLine(last_point, new_point);
                    last_point = new_point;
                }
            }
            float [] new_point = getPolarLineEndpoint(center, radius, end_angle);
            RenderLine(last_point, new_point);
        }
    }
    public void AddPart(String part_name, float[] offset, float bound_width, float bound_height, float minX, float maxX, float minY, float maxY, ViewerPart part)
    {
        part.name = part_name;
        part.offset = offset;
        part.bound_width = bound_width;
        part.bound_height = bound_height;
        part.minX = minX;
        part.maxX = maxX;
        part.minY = minY;
        part.maxY = maxY;
        ViewerPartStack.add(part);
    }
    public void OpenDXFasPart(String filePath, String part_name) throws ParseException {
        Parser parser = ParserBuilder.createDefaultParser();
        parser.parse(filePath, DXFParser.DEFAULT_ENCODING);
        DXFDocument doc = parser.getDocument();
        java.util.List<DXFLine> lst = doc.getDXFLayer("0").getDXFEntities(DXFConstants.ENTITY_TYPE_LINE);
        float minX = 0, maxX = 0, minY = 0, maxY = 0;
        ViewerPart part = new ViewerPart();
        if (lst != null)
        {
            for (int index = 0; index < lst.size(); index++) {
                org.kabeja.dxf.helpers.Point start_point = lst.get(index).getStartPoint();
                org.kabeja.dxf.helpers.Point end_point = lst.get(index).getEndPoint();
                part.addLine(new float[]{(float)start_point.getX(),(float)start_point.getY()}, new float[]{(float)end_point.getX(), (float)end_point.getY()});
                if (lst.get(index).getBounds().getMinimumX() < minX) minX = (float)lst.get(index).getBounds().getMinimumX();
                if (lst.get(index).getBounds().getMaximumX() > maxX) maxX = (float)lst.get(index).getBounds().getMaximumX();
                if (lst.get(index).getBounds().getMinimumY() < minY) minY = (float)lst.get(index).getBounds().getMinimumY();
                if (lst.get(index).getBounds().getMaximumY() > maxY) maxY = (float)lst.get(index).getBounds().getMaximumY();
            }
        }
        java.util.List<DXFArc> arc_lst = doc.getDXFLayer("0").getDXFEntities(DXFConstants.ENTITY_TYPE_ARC);
        if (arc_lst != null)
        {
            for (int index = 0; index < arc_lst.size(); index++) {
                org.kabeja.dxf.helpers.Point start_point = arc_lst.get(index).getStartPoint();
                org.kabeja.dxf.helpers.Point end_point = arc_lst.get(index).getEndPoint();
                org.kabeja.dxf.helpers.Point center_point = arc_lst.get(index).getCenterPoint();
                float radius = (float)arc_lst.get(index).getRadius();
                String direction = "CCW";
                if (arc_lst.get(index).isCounterClockwise())
                {
                    direction = "CW";
                }
                part.addArc(new float[]{(float)start_point.getX(),(float)start_point.getY()}, new float[]{(float)end_point.getX(), (float)end_point.getY()}, new float[]{(float)center_point.getX(), (float)center_point.getY()}, radius, direction);
                if (arc_lst.get(index).getBounds().getMinimumX() < minX) minX = (float)arc_lst.get(index).getBounds().getMinimumX();
                if (arc_lst.get(index).getBounds().getMaximumX() > maxX) maxX = (float)arc_lst.get(index).getBounds().getMaximumX();
                if (arc_lst.get(index).getBounds().getMinimumY() < minY) minY = (float)arc_lst.get(index).getBounds().getMinimumY();
                if (arc_lst.get(index).getBounds().getMaximumY() > maxY) maxY = (float)arc_lst.get(index).getBounds().getMaximumY();
            }
        }
        java.util.List<DXFCircle> circle_lst = doc.getDXFLayer("0").getDXFEntities(DXFConstants.ENTITY_TYPE_CIRCLE);
        if (circle_lst != null)
        {
            for (int index = 0; index < circle_lst.size(); index++) {
                org.kabeja.dxf.helpers.Point center_point = circle_lst.get(index).getCenterPoint();
                float radius = (float)circle_lst.get(index).getRadius();
                part.addArc(new float[]{(float)center_point.getX() + radius,(float)center_point.getY()}, new float[]{(float)center_point.getX() + radius,(float)center_point.getY()}, new float[]{(float)center_point.getX(), (float)center_point.getY()}, radius, "CCW");
                //part.addArc(new float[]{(float)center_point.getX(),(float)center_point.getY() + radius}, new float[]{(float)center_point.getX() - radius,(float)center_point.getY()}, new float[]{(float)center_point.getX(), (float)center_point.getY()}, radius, "CCW");
                //part.addArc(new float[]{(float)center_point.getX() - radius,(float)center_point.getY()}, new float[]{(float)center_point.getX(),(float)center_point.getY() - radius}, new float[]{(float)center_point.getX(), (float)center_point.getY()}, radius, "CCW");
                //part.addArc(new float[]{(float)center_point.getX(),(float)center_point.getY() - radius}, new float[]{(float)center_point.getX() + radius,(float)center_point.getY()}, new float[]{(float)center_point.getX(), (float)center_point.getY()}, radius, "CCW");
                if (circle_lst.get(index).getBounds().getMinimumX() < minX) minX = (float)circle_lst.get(index).getBounds().getMinimumX();
                if (circle_lst.get(index).getBounds().getMaximumX() > maxX) maxX = (float)circle_lst.get(index).getBounds().getMaximumX();
                if (circle_lst.get(index).getBounds().getMinimumY() < minY) minY = (float)circle_lst.get(index).getBounds().getMinimumY();
                if (circle_lst.get(index).getBounds().getMaximumY() > maxY) maxY = (float)circle_lst.get(index).getBounds().getMaximumY();
            }
        }

        float bound_width = maxX - minX;
        float bound_height = maxY - minY;
        //part.addLine(new float[]{minX,minY}, new float[]{maxX, minY});
        //part.addLine(new float[]{maxX,minY}, new float[]{maxX, maxY});
        //part.addLine(new float[]{maxX,maxY}, new float[]{minX, maxY});
        //part.addLine(new float[]{minX,maxY}, new float[]{minX, minY});
        //System.out.println("Bound width: " + bound_width + " Bound height: " + bound_height);
        AddPart(part_name, new float[]{0, 0}, bound_width, bound_height, minX, maxX, minY, maxY, part);
    }
    public void getPaths()
    {
        for(int i = 0; i< ViewerPartStack.size(); i++)
        {
            ViewerPart part = ViewerPartStack.get(i);
            BuildPaths path = new BuildPaths(part.EntityStack); //Feed build paths our DXF entities
            part.paths = path.getPaths(); //return a list of paths in contour chains
            part.tool_paths = new ArrayList(); //Create a new array list to store our offset tool paths
            for (int z = 0; z < part.paths.size(); z++) //Iterate our contour path and buffer it into a tool path
            {
                PathObject tp = new PathObject();
                //System.out.println("Iterating path# - > " + z);
                ArrayList<float[]> points = part.paths.get(z).points;
                Coordinate[] coordinates = new Coordinate[points.size()];
                for (int x = 0; x < points.size(); x++)
                {
                    //System.out.println("\tAdding coordinate point! -> " + new Coordinate(points.get(x)[0], points.get(x)[1], 0) );
                    coordinates[x] = new Coordinate(points.get(x)[0], points.get(x)[1], 0);
                }

                Geometry g = new GeometryFactory().createLineString(coordinates);

                // creates BufferParameters
                BufferParameters bufferParam = new BufferParameters();

                bufferParam.setEndCapStyle(BufferParameters.CAP_ROUND);
                // if using any other parameter result is as expected
                // bufferParam.setEndCapStyle(BufferParameters.CAP_ROUND);
                //bufferParam.setJoinStyle(BufferParameters.JOIN_BEVEL );
                //bufferParam.setMitreLimit(5);
                //bufferParam.setSimplifyFactor(0.01);
                bufferParam.setQuadrantSegments(8);
                //bufferParam.setSingleSided(true);


                // creates buffer geom on point with 10m distance and use set bufferParameters
                Geometry buffer =  BufferOp.bufferOp(g ,0.038f, bufferParam);

                Coordinate[] buffered_path = buffer.getCoordinates();


                ArrayList<float[]> tool_path = new ArrayList();

                int contour_to_find = 2; //Assume we are an inside contour
                if (part.paths.get(z).isOutsideContour == true)
                {
                    contour_to_find = 1; //We are an outside contour
                }
                float[] begin_point = new float[] {0, 0};
                int current_contour = 0;

                if (buffered_path.length > 0)
                {
                    //Calculate Lead in begin point

                    for (int x = 0; x < buffered_path.length; x++)
                    {
                        float[] current_point = new float[]{new Float(buffered_path[x].x), new Float(buffered_path[x].y)};
                        if (x == 0) begin_point = current_point;
                        if (current_contour == contour_to_find) tool_path.add(new float[]{new Float(buffered_path[x].x), new Float(buffered_path[x].y)});
                        if (begin_point[0] == current_point[0] && begin_point[1] == current_point[1]) //We are a closed path
                        {
                            current_contour++;
                        }

                    }

                    //Calculate and add lead in and lead out to top and bottom of stack
                    tp.points = new ArrayList();
                    if (tool_path.size() > 3)
                    {
                        float[] lead_in_point = getPolarLineEndpoint(new float[]{new Float(tool_path.get(0)[0]), new Float(tool_path.get(0)[1])}, 0.030f, getAngle(new float[]{new Float(tool_path.get(0)[0]), new Float(tool_path.get(0)[1])}, new float[]{new Float(tool_path.get(1)[0]), new Float(tool_path.get(1)[1])}) + 140);
                        tp.points.add(lead_in_point);
                        for (int x = 0; x < tool_path.size(); x++) tp.points.add(tool_path.get(x));
                        float[] lead_out_point = getPolarLineEndpoint(new float[]{new Float(tool_path.get(tool_path.size()-1)[0]), new Float(tool_path.get(tool_path.size()-1)[1])}, 0.030f, getAngle(new float[]{new Float(tool_path.get(tool_path.size()-1)[0]), new Float(tool_path.get(tool_path.size()-1)[1])}, new float[]{new Float(tool_path.get(tool_path.size()-2)[0]), new Float(tool_path.get(tool_path.size()-2)[1])}) + 230);
                        tp.points.add(lead_out_point);
                    }
                    part.tool_paths.add(tp);
                }


                //System.out.println("Adding toolpath #" + z);
            }
        }

    }
    public void postProcess(String output_file)
    {
        System.out.println("Posting for material: " + GlobalData.configData.MaterialSelection);
        System.out.println("Posting for consumable: " + GlobalData.configData.ConsumableSelection);

        JetToolpathCutChartData jet_param = new JetToolpathCutChartData();
        for (int x = 0; x < GlobalData.configData.CutChart.size(); x++)
        {
            if (GlobalData.configData.CutChart.get(x).Material.contentEquals(GlobalData.configData.MaterialSelection) &&
                    GlobalData.configData.CutChart.get(x).Consumable.contentEquals(GlobalData.configData.ConsumableSelection))
            {
                jet_param = GlobalData.configData.CutChart.get(x);
            }
        }
        if (jet_param == null)
        {
            System.out.println("Cut Options don't exist!");
            return;
        }
        ArrayList<String> GcodeStack = new ArrayList();
        //Post Gcode Header
        GcodeStack.add("G20 M5 M9");
        GcodeStack.add("G53 G0 Z0"); //Rapid to Z Clearance plane
        getPaths(); //Create toolpaths from contours in each part
        for(int i = 0; i< ViewerPartStack.size(); i++)
        {
            ViewerPart part = ViewerPartStack.get(i);
            if (part.paths != null)
            {
                //Post all contours in this part which are not an outside contour, save that for last
                int outside_contour_index = -1;
                for(int x = 0; x < part.tool_paths.size(); x++)
                {
                    PathObject path = part.tool_paths.get(x);
                    if (path.points.size() > 5) //Ignore random segments left in drawing
                    {
                        if (part.paths.get(x).isOutsideContour == true)
                        {
                            outside_contour_index = x;
                        }
                        else
                        {
                            GcodeStack.add("G0 X" + (path.points.get(0)[0] + part.offset[0]) + " Y" + (path.points.get(0)[1] + part.offset[1])); //Rapid to entry point of contour
                            GcodeStack.add("G38.3 F30 Z-10"); //Probe for top of work piece
                            GcodeStack.add("G91 G0 Z0.220"); //Remove Slack in Floating Head
                            //GcodeStack.add("G10 L20 P1 Z0"); //Set Z=0
                            GcodeStack.add("G91 G0 Z" + jet_param.PierceHeight); //Rapid to pierce height
                            GcodeStack.add("M3S5000"); //Turn Torch on
                            GcodeStack.add("G4 P" + jet_param.PierceDelay); //Pierce Delay
                            GcodeStack.add("G91 G1 F30 Z" + jet_param.CutHeight); //Feed to Cut height
                            GcodeStack.add("M8"); //Turn on ATHC
                            GcodeStack.add("G90"); //Switch back to absolute mode
                            for(int z = 0; z < path.points.size(); z++)
                            {
                                float[] go_point = path.points.get(z);
                                GcodeStack.add("G1 F" + jet_param.Feedrate + " X" + (go_point[0] + part.offset[0]) + " Y" + (go_point[1] + part.offset[1]));
                            }
                            GcodeStack.add("M5"); //Turn Torch off
                            GcodeStack.add("M9"); //Turn ATHC
                            GcodeStack.add("G4 P" + jet_param.PostDelay); //Post Delay
                            GcodeStack.add("G53 G0 Z0"); //Rapid to Z Clearance plane
                        }
                    }
                }
                if (outside_contour_index >= 0)
                {
                    PathObject path = part.tool_paths.get(outside_contour_index);
                    GcodeStack.add("G0 X" + (path.points.get(0)[0] + part.offset[0]) + " Y" + (path.points.get(0)[1] + part.offset[1])); //Rapid to entry point of contour
                    GcodeStack.add("G38.3 F30 Z-10"); //Probe for top of work piece
                    GcodeStack.add("G91 G0 Z0.220"); //Remove Slack in Floating Head
                    //GcodeStack.add("G10 L20 P1 Z0"); //Set Z=0
                    GcodeStack.add("G91 G0 Z" + jet_param.PierceHeight); //Rapid to pierce height
                    GcodeStack.add("M3S5000"); //Turn Torch on
                    GcodeStack.add("G4 P" + jet_param.PierceDelay); //Pierce Delay
                    GcodeStack.add("G91 G1 F30 Z" + jet_param.CutHeight); //Feed to Cut height
                    GcodeStack.add("M8"); //Turn on ATHC
                    GcodeStack.add("G90"); //Switch back to absolute mode
                    for(int z = 0; z < path.points.size(); z++)
                    {
                        float[] go_point = path.points.get(z);
                        GcodeStack.add("G1 F" + jet_param.Feedrate + " X" + (go_point[0] + part.offset[0]) + " Y" + (go_point[1] + part.offset[1]));
                    }
                    GcodeStack.add("M5"); //Turn Torch off
                    GcodeStack.add("M9"); //Turn ATHC
                    GcodeStack.add("G4 P" + jet_param.PostDelay); //Post Delay
                    GcodeStack.add("G53 G0 Z0"); //Rapid to Z Clearance plane
                }
            }
        }
        try {
            GlobalData.writeFile(output_file, GcodeStack);
        } catch (IOException e) {
            e.printStackTrace();
        }

    }
    public void RenderStack(Graphics2D graphics)
    {
        //System.out.println("Begin render!");
        g2d = graphics;
         /* Begin stock boundry outline */
        g2d.setColor(Color.red);
        g2d.draw(new Line2D.Float((0 * GlobalData.ViewerZoom) + GlobalData.ViewerPan[0], ((0 * GlobalData.ViewerZoom) * -1) + GlobalData.ViewerPan[1], (job_stock_size[0] * GlobalData.ViewerZoom) + GlobalData.ViewerPan[0], ((0 * GlobalData.ViewerZoom) * -1) + GlobalData.ViewerPan[1]));
        g2d.draw(new Line2D.Float((job_stock_size[0] * GlobalData.ViewerZoom) + GlobalData.ViewerPan[0], ((0 * GlobalData.ViewerZoom) * -1) + GlobalData.ViewerPan[1], (job_stock_size[0] * GlobalData.ViewerZoom) + GlobalData.ViewerPan[0], ((job_stock_size[1]  * GlobalData.ViewerZoom) * -1) + GlobalData.ViewerPan[1]));
        g2d.draw(new Line2D.Float((job_stock_size[0] * GlobalData.ViewerZoom) + GlobalData.ViewerPan[0], ((job_stock_size[1] * GlobalData.ViewerZoom) * -1) + GlobalData.ViewerPan[1], (0 * GlobalData.ViewerZoom) + GlobalData.ViewerPan[0], ((job_stock_size[1]  * GlobalData.ViewerZoom) * -1) + GlobalData.ViewerPan[1]));
        g2d.draw(new Line2D.Float((0 * GlobalData.ViewerZoom) + GlobalData.ViewerPan[0], ((job_stock_size[1] * GlobalData.ViewerZoom) * -1) + GlobalData.ViewerPan[1], (0 * GlobalData.ViewerZoom) + GlobalData.ViewerPan[0], ((0  * GlobalData.ViewerZoom) * -1) + GlobalData.ViewerPan[1]));
        g2d.setColor(Color.white);
        /* End stock boundry outline */


        for(int i = 0; i< ViewerPartStack.size(); i++)
        {
            ViewerPart part = ViewerPartStack.get(i);
            if (part.engaged == true)
            {
                g2d.setColor(Color.green);
            }
            else
            {
                g2d.setColor(Color.white);
            }
            for(int x = 0; x < part.EntityStack.size(); x++)
            {
                ViewerEntity entity = ViewerPartStack.get(i).EntityStack.get(x);
                if (entity.type == "line")
                {
                    RenderLine(new float[]{entity.start[0] + part.offset[0], entity.start[1] + part.offset[1]}, new float[]{entity.end[0] + part.offset[0], entity.end[1] + part.offset[1]});
                }
                if (entity.type == "cw_arc")
                {
                    //g2d.setColor(Color.red);
                    RenderArc(new float[]{entity.start[0] + part.offset[0], entity.start[1] + part.offset[1]}, new float[]{entity.end[0] + part.offset[0], entity.end[1] + part.offset[1]}, new float[]{entity.center[0] + part.offset[0], entity.center[1] + part.offset[1]}, entity.radius, "CW");
                }
                if (entity.type == "ccw_arc")
                {
                    //g2d.setColor(Color.blue);
                    RenderArc(new float[]{entity.start[0] + part.offset[0], entity.start[1] + part.offset[1]}, new float[]{entity.end[0] + part.offset[0], entity.end[1] + part.offset[1]}, new float[]{entity.center[0] + part.offset[0], entity.center[1] + part.offset[1]}, entity.radius, "CCW");
                }
            }
            if (part.paths != null)
            {
                for(int x = 0; x < part.paths.size(); x++)
                {
                    g2d.setColor(Color.blue);
                    PathObject path = part.paths.get(x);
                    if (path.isClosed == true)
                    {
                        if (path.isOutsideContour == true)
                        {
                            g2d.setColor(Color.cyan);
                        }
                        else
                        {
                            g2d.setColor(Color.blue);
                        }
                    }
                    else
                    {
                        g2d.setColor(Color.orange);
                    }
                    for(int z = 1; z < path.points.size(); z++)
                    {
                        float[] last_point = path.points.get(z-1);
                        float[] current_point = path.points.get(z);
                        RenderLine(new float[]{last_point[0] + part.offset[0], last_point[1] + part.offset[1]}, new float[]{current_point[0] + part.offset[0], current_point[1] + part.offset[1]});
                    }
                }
            }
            if (part.tool_paths != null)
            {
                for(int x = 0; x < part.tool_paths.size(); x++)
                {
                    g2d.setColor(Color.green);
                    PathObject path = part.tool_paths.get(x);
                    for(int z = 1; z < path.points.size(); z++)
                    {
                        float[] last_point = path.points.get(z-1);
                        float[] current_point = path.points.get(z);
                        RenderLine(new float[]{last_point[0] + part.offset[0], last_point[1] + part.offset[1]}, new float[]{current_point[0] + part.offset[0], current_point[1] + part.offset[1]});
                    }
                }
            }
            if (part.engaged == true)
            {
                g2d.setColor(Color.lightGray);
                RenderLine(new float[]{part.minX + part.offset[0], part.minY + part.offset[1]}, new float[]{part.maxX + part.offset[0], part.minY + part.offset[1]});
                RenderLine(new float[]{part.maxX + part.offset[0], part.minY + part.offset[1]}, new float[]{part.maxX + part.offset[0], part.maxY + part.offset[1]});
                RenderLine(new float[]{part.maxX + part.offset[0], part.maxY + part.offset[1]}, new float[]{part.minX + part.offset[0], part.maxY + part.offset[1]});
                RenderLine(new float[]{part.minX + part.offset[0], part.maxY + part.offset[1]}, new float[]{part.minX + part.offset[0], part.minY + part.offset[1]});
            }
        }
    }
    public void ClearStack()
    {
        ViewerPartStack.clear();
    }
    public void ClickPressStack(float mousex, float mousey){
        isMousePressed = true;
        for (int x = 0; x < ViewerPartStack.size(); x++)
        {
            if ((mousex > ViewerPartStack.get(x).offset[0] + ViewerPartStack.get(x).minX && mousex < ViewerPartStack.get(x).offset[0] + ViewerPartStack.get(x).maxX) && (mousey > ViewerPartStack.get(x).offset[1] + ViewerPartStack.get(x).minY && mousey < ViewerPartStack.get(x).offset[1] + ViewerPartStack.get(x).maxY))
            {
                ViewerPartStack.get(x).engaged = true;
                return; //Only click on first selection
            }
        }
    }
    public void ClickReleaseStack(float mousex, float mousey){
        if (isMousePressed == true)
        {
            isMousePressed = false;
            for (int x = 0; x < ViewerPartStack.size(); x++)
            {
                if ((mousex > ViewerPartStack.get(x).offset[0] + ViewerPartStack.get(x).minX && mousex < ViewerPartStack.get(x).offset[0] + ViewerPartStack.get(x).maxX) && (mousey > ViewerPartStack.get(x).offset[1] + ViewerPartStack.get(x).minY && mousey < ViewerPartStack.get(x).offset[1] + ViewerPartStack.get(x).maxY))
                {
                    ViewerPartStack.get(x).engaged = false;
                    return;
                }
            }
        }
    }
    public void MouseMotionStack(float mousex, float mousey){
        if (isMousePressed == true)
        {
            for (int x = 0; x < ViewerPartStack.size(); x++)
            {
                if (ViewerPartStack.get(x).engaged == true)
                {
                    float move_x = mousex - mouseLastDragPosition[0];
                    float move_y = mousey - mouseLastDragPosition[1];
                    ViewerPartStack.get(x).offset[0] += move_x;
                    ViewerPartStack.get(x).offset[1] += move_y;
                    break;
                }
            }
        }
        mouseLastDragPosition = new float[]{mousex, mousey};
    }
    public void UpdateBoundsOfPart(float[] pivot_point, int index)
    {
        ViewerPart part = ViewerPartStack.get(index);
        float minX = pivot_point[0];
        float maxX = pivot_point[0];
        float minY = pivot_point[1];
        float maxY = pivot_point[1];
        for (int i = 0; i < part.EntityStack.size(); i++)
        {
            ViewerEntity entity = part.EntityStack.get(i);
            if (entity.type == "line")
            {
                if (entity.start[0] < minX) minX = entity.start[0];
                if (entity.start[0] > maxX) maxX = entity.start[0];
                if (entity.start[1] < minY) minY = entity.start[1];
                if (entity.start[1] > maxY) maxY = entity.start[1];

                if (entity.end[0] < minX) minX = entity.end[0];
                if (entity.end[0] > maxX) maxX = entity.end[0];
                if (entity.end[1] < minY) minY = entity.end[1];
                if (entity.end[1] > maxY) maxY = entity.end[1];
            }
            if (entity.type == "cw_arc" || entity.type == "ccw_arc")
            {
                if (entity.start[0] < minX) minX = entity.start[0];
                if (entity.start[0] > maxX) maxX = entity.start[0];
                if (entity.start[1] < minY) minY = entity.start[1];
                if (entity.start[1] > maxY) maxY = entity.start[1];

                if (entity.end[0] < minX) minX = entity.end[0];
                if (entity.end[0] > maxX) maxX = entity.end[0];
                if (entity.end[1] < minY) minY = entity.end[1];
                if (entity.end[1] > maxY) maxY = entity.end[1];

                if (entity.center[0] < minX) minX = entity.center[0];
                if (entity.center[0] > maxX) maxX = entity.center[0];
                if (entity.center[1] < minY) minY = entity.center[1];
                if (entity.center[1] > maxY) maxY = entity.center[1];
            }
        }
        part.minX = minX;
        part.maxX = maxX;
        part.minY = minY;
        part.maxY = maxY;
    }
    public void RotateEngagedPart(float inc_degrees)
    {
        for (int x = 0; x < ViewerPartStack.size(); x++)
        {
            if (ViewerPartStack.get(x).engaged == true)
            {
                ViewerPart part = ViewerPartStack.get(x);
                part.rotation_angle += inc_degrees;
                float[] pivot_point = getMidpoint(new float[] {part.minX, part.minY}, new float[] {part.maxX, part.maxY});
                for (int i = 0; i < part.EntityStack.size(); i++)
                {
                    ViewerEntity entity = part.EntityStack.get(i);
                    if (entity.type == "line")
                    {
                        rotatePoint(pivot_point, entity.start, inc_degrees);
                        rotatePoint(pivot_point, entity.end, inc_degrees);
                    }
                    if (entity.type == "cw_arc" || entity.type == "ccw_arc")
                    {
                        rotatePoint(pivot_point, entity.start, inc_degrees);
                        rotatePoint(pivot_point, entity.end, inc_degrees);
                        rotatePoint(pivot_point, entity.center, inc_degrees);
                    }
                }
                UpdateBoundsOfPart(pivot_point, x);
                break;
            }
        }
    }
}
