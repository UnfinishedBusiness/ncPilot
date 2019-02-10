package com.nc.pilot.lib;

import java.awt.*;
import java.awt.geom.Line2D;
import java.util.ArrayList;

/**
 * Created by admin on 2/1/19.
 */
public class ToolpathViewer {
    public class ViewerEntity{
        public String type;
        public float[] start;
        public float[] end;
        public float[] center;
        public float radius;
    }
    private float[] job_stock_size = new float[] {20, 20};

    private Graphics2D g2d;
    private ArrayList<ViewerEntity> toolpathViewerStack = new ArrayList();


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
    public void addArc(float[] start, float[] end, float[] center, float radius, String direction) {
        ViewerEntity e = new ViewerEntity();
        if (direction == "CW")
        {
            e.type = "cw_arc";
            e.start = start;
            e.end = end;
            e.radius = radius;
        }
        if (direction == "CCW")
        {
            e.type = "ccw_arc";
            e.start = start;
            e.end = end;
            e.radius = radius;
        }
        e.center = center;
        toolpathViewerStack.add(e);
    }
    public void addLine(float[] start, float[] end) {
        ViewerEntity e = new ViewerEntity();
        e.type = "line";
        e.start = start;
        e.end = end;
        toolpathViewerStack.add(e);
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


        for(int i = 0; i< toolpathViewerStack.size(); i++)
        {
            ViewerEntity entity = toolpathViewerStack.get(i);
            if (entity.type == "line") //We are a line move
            {
                g2d.setColor(Color.white);
                RenderLine(entity.start, entity.end);
            }
            if (entity.type == "cw_arc") //We are a clockwise arc
            {
                g2d.setColor(Color.white);
                RenderArc(entity.start, entity.end, entity.center, entity.radius, "CW");
            }
            if (entity.type == "ccw_arc") //We are a counter-clockwise arc
            {
                g2d.setColor(Color.white);
                RenderArc(entity.start, entity.end, entity.center, entity.radius, "CCW");
            }
        }
    }
    public void ClearStack()
    {
        toolpathViewerStack.clear();
    }
}
