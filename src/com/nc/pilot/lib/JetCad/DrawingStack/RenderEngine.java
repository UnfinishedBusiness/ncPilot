package com.nc.pilot.lib.JetCad.DrawingStack;

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
import java.beans.XMLDecoder;
import java.beans.XMLEncoder;
import java.io.*;
import java.util.ArrayList;

/**
 * Created by travis on 2/1/19.
 */

public class RenderEngine {

    private Graphics2D g2d;
    public ArrayList<DrawingEntity> DrawingStack = new ArrayList();
    private boolean isMousePressed = false;
    private float[] mouseLastDragPosition;

    // constructor
    public RenderEngine() {

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
        g2d.draw(new Line2D.Float(((start[0]) * GlobalData.ViewerZoom) + GlobalData.ViewerPan[0], (((start[1]) * GlobalData.ViewerZoom) * -1), ((end[0]) * GlobalData.ViewerZoom), (((end[1]) * GlobalData.ViewerZoom) * -1) + GlobalData.ViewerPan[1]));
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
    public void RenderStack(Graphics2D graphics)
    {
        //System.out.println("Begin render!");
        g2d = graphics;
         /* Begin stock boundry outline */
        g2d.setColor(Color.red);
        g2d.draw(new Line2D.Float(0 + GlobalData.ViewerPan[0], 0 + GlobalData.ViewerPan[1], GlobalData.ViewerPan[0] + 10, GlobalData.ViewerPan[1] + 0));
        g2d.draw(new Line2D.Float(0 + GlobalData.ViewerPan[0], 0 + GlobalData.ViewerPan[1], GlobalData.ViewerPan[0] + 0, GlobalData.ViewerPan[1] + 10));
        g2d.draw(new Line2D.Float(0 + GlobalData.ViewerPan[0], 0 + GlobalData.ViewerPan[1], GlobalData.ViewerPan[0] - 10, GlobalData.ViewerPan[1] + 0));
        g2d.draw(new Line2D.Float(0 + GlobalData.ViewerPan[0], 0 + GlobalData.ViewerPan[1], GlobalData.ViewerPan[0] + 0, GlobalData.ViewerPan[1] - 10));
        g2d.setColor(Color.green);
        /* End stock boundry outline */


        for(int i = 0; i< DrawingStack.size(); i++) {
            //System.out.println("Rendering part " + i);
            DrawingEntity entity = DrawingStack.get(i);
            if (entity.type.contentEquals("line"))
            {
                //System.out.println("Rendering line -> " + new float[]{entity.start[0] + part.offset[0], entity.start[1] + part.offset[1]} + " - " + new float[]{entity.end[0] + part.offset[0], entity.end[1] + part.offset[1]});
                RenderLine(new float[]{entity.start[0], entity.start[1]}, new float[]{entity.end[0], entity.end[1]});
            }
            if (entity.type.contentEquals("cw_arc"))
            {
                //g2d.setColor(Color.red);
                RenderArc(new float[]{entity.start[0], entity.start[1]}, new float[]{entity.end[0], entity.end[1]}, new float[]{entity.center[0], entity.center[1]}, entity.radius, "CW");
            }
            if (entity.type.contentEquals("ccw_arc"))
            {
                //g2d.setColor(Color.blue);
                RenderArc(new float[]{entity.start[0], entity.start[1]}, new float[]{entity.end[0], entity.end[1]}, new float[]{entity.center[0], entity.center[1]}, entity.radius, "CCW");
            }
        }
    }
    public void ClickPressStack(float mousex, float mousey){
        isMousePressed = true;
        for (int x = 0; x < DrawingStack.size(); x++)
        {

        }
    }
    public void ClickReleaseStack(float mousex, float mousey){
        if (isMousePressed == true)
        {
            isMousePressed = false;
            for (int x = 0; x < DrawingStack.size(); x++)
            {

            }
        }
    }
    public void MouseMotionStack(float mousex, float mousey){
        if (isMousePressed == true)
        {
            for (int x = 0; x < DrawingStack.size(); x++)
            {

            }
        }
        mouseLastDragPosition = new float[]{mousex, mousey};
    }
}
