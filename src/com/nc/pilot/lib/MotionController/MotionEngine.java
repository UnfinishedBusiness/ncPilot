package com.nc.pilot.lib.MotionController;

import com.nc.pilot.lib.GcodeInterpreter;

import java.util.ArrayList;

/**
 * Created by travi on 2/22/2019.
 */
public class MotionEngine {

    private ArrayList<GcodeInterpreter.GcodeMove> moves;

    private static long one_minute = 60000000; //60,000,000

    private long[] step_scale;


    public MotionEngine(){
        step_scale = new long[] {650, 650, 650};
    }

    private long[] getIndividualAxisFeedrates(float f, float x_dist, float y_dist, float z_dist)
    {
        return new long[] {(long)(one_minute / (x_dist * step_scale[0]) / f), (long)(one_minute / (y_dist * step_scale[1]) / f), (long)(one_minute / (z_dist * step_scale[2]) / f)};
    }

    public void setMoves(ArrayList<GcodeInterpreter.GcodeMove> m)
    {
        moves = m;
    }

    public void test()
    {
        for (int x = 1; x < moves.size(); x ++)
        {
            if (moves.get(x).Gword == 1)
            {
                float x_dist = (moves.get(x).Xword - moves.get(x-1).Xword);
                float y_dist = (moves.get(x).Yword - moves.get(x-1).Yword);
                float z_dist = (moves.get(x).Zword - moves.get(x-1).Zword);
                long[] feedrates = getIndividualAxisFeedrates(moves.get(x).Fword, x_dist, y_dist, z_dist);
                String send_buf = ">" + (long)(x_dist * step_scale[0]) + ":" + feedrates[0] + "|" +  (long)(y_dist * step_scale[1]) + ":" + feedrates[1] + "|" + (long)(z_dist * step_scale[2]) + ":" + feedrates[2] + "\n";
                System.out.print(send_buf);
            }
            if (moves.get(x).Gword == 2)
            {

            }
            if (moves.get(x).Gword == 3)
            {

            }
        }
    }
}
