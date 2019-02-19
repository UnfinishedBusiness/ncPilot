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
            if (inTolerance(p[0], e.start[0], point_tolorance) && inTolerance(p[1], e.start[1], point_tolorance))
            {
                //System.out.println("Next point is the start_point of entity: " + x);
                current_path.points.add(e.end);
                UsedIndexes.add(indexes.get(x));
                return true;
            }
            else if (inTolerance(p[0], e.end[0], point_tolorance) && inTolerance(p[1], e.end[1], point_tolorance))
            {
                //System.out.println("Next point is the end_point of entity: " + x);
                current_path.points.add(e.start);
                UsedIndexes.add(indexes.get(x));
                return true;
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
                PathStack.add(current_path);
            }
            else
            {
                break; //All paths have been found!
            }
        }

    }
}
