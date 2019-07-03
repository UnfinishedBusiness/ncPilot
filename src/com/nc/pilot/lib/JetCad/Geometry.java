package com.nc.pilot.lib.JetCad;
import com.nc.pilot.lib.JetCad.DrawingStack.DrawingEntity;
import com.nc.pilot.lib.JetCad.DrawingStack.RenderEngine;

import java.awt.*;
import java.util.ArrayList;

public class Geometry {
    RenderEngine render_engine;
    public Geometry(RenderEngine r)
    {
        render_engine = r;
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
    public ArrayList<float[]> getArcPoints(float[] start, float[] end, float[] center, float radius, String direction)
    {
        ArrayList<float[]> points = new ArrayList();
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
                points.add(new_point);
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
                    points.add(new_point);
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
                    points.add(new_point);
                    last_point = new_point;
                }
            }
            float [] new_point = getPolarLineEndpoint(center, radius, end_angle);
        }
        return points;
    }
    public ArrayList<float[]> getIntersectionPoints(ArrayList<DrawingEntity> e)
    {
        ArrayList<float[]> ret = new ArrayList();
        for (int x = 0; x < e.size(); x++)
        {
            for (int y = 0; y < e.size(); y++)
            {
                if (e.get(x).type.contentEquals("line") && e.get(y).type.contentEquals("line"))
                {
                    System.out.println("Getting line<->line intersection!");
                    DrawingEntity line1 = e.get(x);
                    DrawingEntity line2 = e.get(y);
                    float a1 = line1.end[1] - line1.start[1];
                    float b1 = line1.start[0] - line1.end[0];
                    float c1 = a1 * line1.start[0] + b1 * line1.start[1];
                    float a2 = line2.end[1] - line2.start[1];
                    float b2 = line2.start[0] - line2.end[0];
                    float c2 = a2 * line2.start[0] + b2 * line2.start[1];
                    float det = a1 * b2 - a2 * b1;
                    if (det == 0) break; //No intersection point, lines are parallel!
                    float x_int = (b2 * c1 - b1 * c2) / det;
                    float y_int = (a1 * c2 - a2 * c1) / det;
                    ret.add(new float[]{x_int, y_int});
                }
                if (e.get(x).type.contentEquals("line") && e.get(y).type.contentEquals("cw_arc"))
                {
                    System.out.println("Getting line<->arc intersection!");
                    DrawingEntity line = e.get(x);
                    DrawingEntity arc = e.get(y);
                    ArrayList<float[]> arc_points = getArcPoints(arc.start, arc.end, arc.center, arc.radius, "CW");
                    if (arc_points.size() > 0)
                    {
                        float[] last_point = arc_points.get(0);
                        for (int z = 1; z < arc_points.size(); z++)
                        {
                            DrawingEntity line1 = line;
                            DrawingEntity line2 = new DrawingEntity();
                            line2.type = "line";
                            line2.start = last_point;
                            line2.end = arc_points.get(z);
                            line2.color = Color.green;
                            render_engine.DrawingStack.add(line2);
                            last_point = arc_points.get(z);
                            float a1 = line1.end[1] - line1.start[1];
                            float b1 = line1.start[0] - line1.end[0];
                            float c1 = a1 * line1.start[0] + b1 * line1.start[1];
                            float a2 = line2.end[1] - line2.start[1];
                            float b2 = line2.start[0] - line2.end[0];
                            float c2 = a2 * line2.start[0] + b2 * line2.start[1];
                            float det = a1 * b2 - a2 * b1;
                            if (det == 0) break; //No intersection point, lines are parallel!
                            float x_int = (b2 * c1 - b1 * c2) / det;
                            float y_int = (a1 * c2 - a2 * c1) / det;
                            ret.add(new float[]{x_int, y_int});
                        }
                    }
                }
            }
        }
        return ret;
    }
}
