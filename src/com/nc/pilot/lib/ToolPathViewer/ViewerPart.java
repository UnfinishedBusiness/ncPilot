package com.nc.pilot.lib.ToolPathViewer;

import java.util.ArrayList;

/**
 * Created by admin on 2/9/19.
 */
public class ViewerPart{
    public ArrayList<ViewerEntity> EntityStack = new ArrayList();
    float[] offset;
    float bound_width;
    float bound_height;
    float scale;
    float minX;
    float maxX;
    float minY;
    float maxY;
    float rotation_angle;
    String name;
    boolean engaged;
    ArrayList<PathObject>  paths;
    ArrayList<PathObject>  tool_paths;
    public ViewerPart(){

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
        EntityStack.add(e);
    }
    public void addLine(float[] start, float[] end) {
        ViewerEntity e = new ViewerEntity();
        e.type = "line";
        e.start = start;
        e.end = end;
        EntityStack.add(e);
    }
}