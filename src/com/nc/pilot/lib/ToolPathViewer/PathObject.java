package com.nc.pilot.lib.ToolPathViewer;

import java.util.ArrayList;

/**
 * Created by admin on 2/18/19.
 */
public class PathObject {
    public ArrayList<float[]> points = new ArrayList();
    boolean isClosed;
    boolean isOutsideContour;
    float chainLength;
}
