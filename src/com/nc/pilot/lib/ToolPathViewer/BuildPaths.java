package com.nc.pilot.lib.ToolPathViewer;

import java.util.ArrayList;

/**
 * Created by travis on 2/18/19.
 */
public class BuildPaths {
    float point_tolorance = 0.001f;
    private ArrayList<ViewerEntity> EntityStack = new ArrayList();
    private ArrayList<PathObject> PathStack = new ArrayList();
    private ArrayList<Integer> UsedIndexes = new ArrayList();
    private PathObject current_path;

    private boolean inTolerance(float a, float b, float t)
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
        if (diff <= Math.abs(t) && diff >= -Math.abs(t))
        {
            return true;
        }
        else
        {
            return false;
        }
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
    public float[] getMidpoint(float[] start_point, float[] end_point) {
        return new float[] {(end_point[0] + start_point[0])/2,(end_point[1] + start_point[1])/2};
    }
    public float getLineLength(float[] start_point, float[] end_point)
    {
        return new Float(Math.hypot(start_point[0]-end_point[0], start_point[1]-end_point[1]));
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
    public ArrayList<float[]> getPointsOfArc(float[] start, float[] end, float[] center, float radius, String direction)
    {
        float start_angle = getAngle(center, start);
        float end_angle = getAngle(center, end);
        float angle_inc = 1;
        ArrayList<float[]> points = new ArrayList();
        points.add(start);
        //System.out.println("start_angle: " + start_angle + " end_angle: " + end_angle);
        if (start_angle == end_angle) //We are a circle
        {
            for (float x = 0; x < 360; x += angle_inc)
            {
                start_angle += angle_inc;
                float [] new_point = getPolarLineEndpoint(center, radius, start_angle);
                points.add(new_point);
            }
        }
        else
        {
            if (direction == "CW")
            {
                for (int x = 0; x < 400; x++) //Runaway protection!
                {
                    start_angle -= angle_inc;
                    if (start_angle <= 0)
                    {
                        start_angle = 360;
                    }
                    else if (inTolerance(start_angle, end_angle, angle_inc * 2))
                    {
                        break; //End of arc, break loop!
                    }
                    float [] new_point = getPolarLineEndpoint(center, radius, start_angle);
                    points.add(new_point);
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
                    points.add(new_point);
                }
            }
            float [] new_point = getPolarLineEndpoint(center, radius, end_angle);
            points.add(new_point);
        }
        return points;
    }
    private boolean hasEntityBeenUsed(int i)
    {
        for (int x = 0; x < UsedIndexes.size(); x++)
        {
            if (UsedIndexes.get(x) == i)
            {
                return true;
            }
        }
        return false;
    }
    private ArrayList<Integer> getUnusedIndexes()
    {
        ArrayList<Integer> r = new ArrayList();
        for (int x = 0; x < EntityStack.size(); x++)
        {
            if (hasEntityBeenUsed(x) == false) r.add(x);
        }
        return r;
    }
    private boolean getNextPointOfCurrentPath(float p[])
    {
        ArrayList<Integer> indexes = getUnusedIndexes();
        for (int x = 0; x < indexes.size(); x++)
        {
            ViewerEntity e = EntityStack.get(indexes.get(x));
            if (e.type == "line")
            {
                if (inTolerance(p[0], e.start[0], point_tolorance) && inTolerance(p[1], e.start[1], point_tolorance))
                {
                    current_path.points.add(e.end);
                    UsedIndexes.add(indexes.get(x));
                    return true;
                }
                else if (inTolerance(p[0], e.end[0], point_tolorance) && inTolerance(p[1], e.end[1], point_tolorance))
                {
                    current_path.points.add(e.start);
                    UsedIndexes.add(indexes.get(x));
                    return true;
                }
            }
            else if (e.type == "ccw_arc")
            {
                if (inTolerance(p[0], e.start[0], point_tolorance) && inTolerance(p[1], e.start[1], point_tolorance))
                {
                    ArrayList<float[]> arc_points = getPointsOfArc(e.start, e.end, e.center, e.radius, "CCW");
                    for (int y = 0; y < arc_points.size(); y++)
                    {
                        current_path.points.add(arc_points.get(y));
                    }
                    UsedIndexes.add(indexes.get(x));
                    return true;
                }
                else if (inTolerance(p[0], e.end[0], point_tolorance) && inTolerance(p[1], e.end[1], point_tolorance))
                {
                    ArrayList<float[]> arc_points = getPointsOfArc(e.end, e.start, e.center, e.radius, "CW");
                    for (int y = 0; y < arc_points.size(); y++)
                    {
                        current_path.points.add(arc_points.get(y));
                    }
                    UsedIndexes.add(indexes.get(x));
                    return true;
                }
            }
            else if (e.type == "cw_arc")
            {
                if (inTolerance(p[0], e.start[0], point_tolorance) && inTolerance(p[1], e.start[1], point_tolorance))
                {
                    ArrayList<float[]> arc_points = getPointsOfArc(e.start, e.end, e.center, e.radius, "CW");
                    for (int y = 0; y < arc_points.size(); y++)
                    {
                        current_path.points.add(arc_points.get(y));
                    }
                    UsedIndexes.add(indexes.get(x));
                    return true;
                }
                else if (inTolerance(p[0], e.end[0], point_tolorance) && inTolerance(p[1], e.end[1], point_tolorance))
                {
                    ArrayList<float[]> arc_points = getPointsOfArc(e.end, e.start, e.center, e.radius, "CCW");
                    for (int y = 0; y < arc_points.size(); y++)
                    {
                        current_path.points.add(arc_points.get(y));
                    }
                    UsedIndexes.add(indexes.get(x));
                    return true;
                }
            }

        }
        return false;
    }
    public ArrayList<PathObject> getPaths()
    {
        return PathStack;
    }
    public BuildPaths(ArrayList<ViewerEntity> stack)
    {
        EntityStack = stack;
        while(true)
        {
            ArrayList<Integer> indexes = getUnusedIndexes();
            if (indexes.size() > 0)
            {
                ViewerEntity e = EntityStack.get(indexes.get(0)); //Get first entity that has not been used
                UsedIndexes.add(indexes.get(0));
                current_path = new PathObject();
                current_path.points.add(e.start); //Prime current path with start point of first not-used entity
                current_path.points.add(e.end); //Prime current path with end point of first not-used entity to start the vector
                while(true)
                {
                    if (getNextPointOfCurrentPath(current_path.points.get(current_path.points.size()-1)) == false)
                    {
                        break; //End of path, determine if we are open or closed and push to PathStack
                    }
                }
                if (current_path.points.size() > 0)
                {
                    if (inTolerance(e.start[0], current_path.points.get(current_path.points.size()-1)[0], point_tolorance) && inTolerance(e.start[1], current_path.points.get(current_path.points.size()-1)[1], point_tolorance))
                    {
                        current_path.isClosed = true;
                        //System.out.println("Path is closed!");
                    }
                    else
                    {
                        current_path.isClosed = false;
                        //System.out.println("Path is open!");
                    }
                }
                PathStack.add(current_path);
            }
            else
            {
                break; //All paths have been found!
            }
        }
        //Calculate chain lengths of each path
        float bigest_chain_length = 0;
        int bigest_index = 0;
        for (int x = 0; x < PathStack.size(); x++)
        {
            PathObject path = PathStack.get(x);
            for (int y = 1; y < path.points.size(); y++)
            {
                path.chainLength += getLineLength(path.points.get(y-1), path.points.get(y));
            }
            if (path.chainLength > bigest_chain_length)
            {
                bigest_chain_length = path.chainLength;
                bigest_index = x;
            }
            //System.out.println("Chain length: " + path.chainLength);
        }
        PathStack.get(bigest_index).isOutsideContour = true;
    }
}
