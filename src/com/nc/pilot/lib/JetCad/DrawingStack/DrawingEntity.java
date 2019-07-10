package com.nc.pilot.lib.JetCad.DrawingStack;

import java.awt.*;

/**
 * Created by Travis on 2/9/19.
 */
public class DrawingEntity {
    public String type;
    public float[] start;
    public float[] end;
    public float[] center;
    public float radius;

    //Special Data
    public float[] intersection_point;
    public boolean has_intersection_point;

    //Meta Data
    public boolean isSelected;
    public Color color = Color.BLACK;
}
