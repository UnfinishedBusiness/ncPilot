package com.nc.pilot.lib.ToolPathViewer;

import com.nc.pilot.lib.GlobalData;
import org.kabeja.dxf.*;
import org.kabeja.parser.DXFParser;
import org.kabeja.parser.ParseException;
import org.kabeja.parser.Parser;
import org.kabeja.parser.ParserBuilder;

import java.awt.*;
import java.awt.geom.Line2D;
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
    public void RenderLine(float[] start, float end[])
    {
        g2d.draw(new Line2D.Float(((start[0] + GlobalData.work_offset[0]) * GlobalData.ViewerZoom) + GlobalData.ViewerPan[0], (((start[1] + GlobalData.work_offset[1]) * GlobalData.ViewerZoom) * -1) + GlobalData.ViewerPan[1], ((end[0] + GlobalData.work_offset[0]) * GlobalData.ViewerZoom) + GlobalData.ViewerPan[0], (((end[1] + GlobalData.work_offset[1]) * GlobalData.ViewerZoom) * -1) + GlobalData.ViewerPan[1]));
    }
    public void RenderArc(float[] start, float[] end, float[] center, float radius, String direction)
    {
        float start_angle = getAngle(center, start);
        float end_angle = getAngle(center, end);
        float angle_inc = 1;
        float[] last_point = start;
        //System.out.println("start_angle: " + start_angle + " end_angle: " + end_angle);
        if (start_angle == end_angle) //We are a circle
        {
            for (float x = 0; x < 360; x += angle_inc)
            {
                start_angle += angle_inc;
                float [] new_point = getPolarLineEndpoint(center, radius, start_angle);
                RenderLine(last_point, new_point);
                last_point = new_point;
            }
        }
        else
        {
            if (direction == "CW")
            {
                for (int x = 0; x < 400; x++) //Runaway protection!
                {
                    start_angle -= angle_inc;
                    //System.out.println("current_angle: " + start_angle + " end_angle: " + end_angle);
                    if (start_angle <= 0)
                    {
                        start_angle = 360;
                    }
                    else if (inTolerance(start_angle, end_angle, angle_inc * 2))
                    {
                        //System.out.println("Found Endpoint!");
                        break; //End of arc, break loop!
                    }
                    float [] new_point = getPolarLineEndpoint(center, radius, start_angle);
                    RenderLine(last_point, new_point);
                    last_point = new_point;
                    if (x == 399)
                    {
                        //System.out.println("Missed endpoint on Clockwise arc! start_angle: " + start_angle + " end_angle: " + end_angle);
                    }
                }
            }
            else
            {
                for (int x = 0; x < 400; x++) //Runaway protection!
                {
                    start_angle += angle_inc;
                    if (start_angle >= 360)
                    {
                        start_angle = 0;
                    }
                    else if (inTolerance(start_angle, end_angle, angle_inc * 2)) break; //End of arc, break loop!
                    float [] new_point = getPolarLineEndpoint(center, radius, start_angle);
                    RenderLine(last_point, new_point);
                    last_point = new_point;
                    if (x == 399)
                    {
                        //System.out.println("Missed endpoint on Counter-Clockwise arc! start_angle: " + start_angle + " end_angle: " + end_angle);
                    }
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
            BuildPaths path = new BuildPaths(part.EntityStack);
            part.paths = path.getPaths();
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
                    RenderArc(new float[]{entity.start[0] + part.offset[0], entity.start[1] + part.offset[1]}, new float[]{entity.end[0] + part.offset[0], entity.end[1] + part.offset[1]}, new float[]{entity.center[0] + part.offset[0], entity.center[1] + part.offset[1]}, entity.radius, "CW");
                }
                if (entity.type == "ccw_arc")
                {
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
