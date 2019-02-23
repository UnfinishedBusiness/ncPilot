package com.nc.pilot.lib.MotionController;

import com.nc.pilot.lib.GcodeInterpreter;
import com.nc.pilot.lib.GlobalData;

import java.util.ArrayList;

/**
 * Created by travi on 2/22/2019.
 */
public class MotionEngine {

    private ArrayList<GcodeInterpreter.GcodeMove> moves;
    private ArrayList<StepGenStruct> move_buffer;
    private float[] move_dro_position;
    private float[] motion_dro_before_move;

    private static int step_len = 10; //this has to match what the step gen is set too!
    private static int motion_buffer_size = 40; //This has to match what is in the step gen controller!
    private static long one_minute = 60000000; //60,000,000
    private static long x_timer;
    private static long y_timer;
    private static long z_timer;

    private long[] step_scale;
    private float max_linear_velocity = 200;

    public MotionEngine(){
        move_buffer = new ArrayList();
        step_scale = new long[] {650, 650, 650};

        x_timer = micros();
        y_timer = micros();
        z_timer = micros();

        GlobalData.dro[0] = 0;
        GlobalData.dro[1] = 0;
        GlobalData.dro[2] = 0;
    }
    private long micros()
    {
        return System.nanoTime() / 500;
    }

    private long[] getIndividualAxisFeedrates(float f, float x_dist, float y_dist, float z_dist)
    {
        long x_feed;
        long y_feed;
        long z_feed;
        if (Math.abs(x_dist) > 0)
        {
            x_feed = (long)(one_minute / (Math.abs(x_dist) * step_scale[0]) / f);
        }
        else
        {
            x_feed = 0;
        }
        if (Math.abs(y_dist) > 0)
        {
            y_feed = (long)(one_minute / (Math.abs(y_dist) * step_scale[1]) / f);
        }
        else
        {
            y_feed = 0;
        }
        if (Math.abs(z_dist) > 0)
        {
            z_feed = (long)(one_minute / (Math.abs(z_dist) * step_scale[2]) / f);
        }
        else
        {
            z_feed = 0;
        }
        return new long[] {x_feed, y_feed, z_feed};
    }

    public void setMoves(ArrayList<GcodeInterpreter.GcodeMove> m)
    {
        moves = m;
    }
    private StepGenStruct adjustStepsForAcuracyDeviation(StepGenStruct s)
    {
        //Figure out which axis is traveling the most
        /*System.out.println("X Deviation: " + (s.x_delay * s.x_step_count));
        System.out.println("Y Deviation: " + (s.y_delay * s.y_step_count));
        if ((s.x_delay * s.x_step_count) > 0 && (s.y_delay * s.y_step_count) > 0) //Make sure we are a linear interpolation
        {
            if ((s.x_delay * s.x_step_count) < (s.y_delay * s.y_step_count))
            {
                int correction_amount = 0;
                while((s.x_delay * s.x_step_count) <= (s.y_delay * s.y_step_count))
                {
                    s.y_delay--;
                    System.out.println("Added y step delay: " + s.y_step_count);
                    correction_amount++;
                }
                System.out.println("Corrected by: " + correction_amount);
            }
            else //Min deviation axis is y
            {
                int correction_amount = 0;
                while((s.y_delay * s.y_step_count) <= (s.x_delay * s.x_step_count))
                {
                    s.x_delay--;
                    System.out.println((s.y_delay * s.y_step_count) + " Added x step delay: " + (s.x_delay * s.x_step_count));
                    correction_amount++;
                }
               System.out.println("Corrected by: " + correction_amount);
            }
        }*/

        return s;
    }
    private long getStepCountWithDirection(int axis, StepGenStruct s)
    {
        long ret = 0;
        if (axis == 0)
        {
            if (s.x_dir == true)
            {
                ret = s.x_step_count;
            }
            else
            {
                ret = s.x_step_count * -1;
            }
        }
        if (axis == 1)
        {
            if (s.y_dir == true)
            {
                ret = s.y_step_count;
            }
            else
            {
                ret = s.y_step_count * -1;
            }
        }
        if (axis == 2)
        {
            if (s.z_dir == true)
            {
                ret = s.z_step_count;
            }
            else
            {
                ret = s.z_step_count * -1;
            }
        }
        return ret;
    }
    private void pushMoveToStack(StepGenStruct s)
    {
        StepGenStruct a = adjustStepsForAcuracyDeviation(s);
        String send_buf = ">" + getStepCountWithDirection(0, a) + ":" + a.x_delay + "|" +  getStepCountWithDirection(1, a) + ":" + a.y_delay + "|" + getStepCountWithDirection(2, a) + ":" + a.z_delay + "\n";
        System.out.print(send_buf);
        MotionController.WriteBuffer(send_buf);
        move_buffer.add(s);
    }
    public void runMoves()
    {
        move_dro_position = new float[] {GlobalData.dro[0], GlobalData.dro[1], GlobalData.dro[2]};
        for (int x = 0; x < moves.size(); x ++)
        {
            if (moves.get(x).Gword == 0)
            {
                float x_dist = (moves.get(x).Xword - move_dro_position[0]);
                float y_dist = (moves.get(x).Yword - move_dro_position[1]);
                float z_dist = (moves.get(x).Zword - move_dro_position[2]);
                //System.out.println("x_dist: " + x_dist);
                //System.out.println("y_dist: " + y_dist);
                long[] feedrates = getIndividualAxisFeedrates(max_linear_velocity, x_dist, y_dist, z_dist);
                StepGenStruct gen = new StepGenStruct();
                gen.x_step_count = (long)(Math.abs(x_dist) * (float)step_scale[0]);
                gen.x_total_step_count = gen.x_step_count;
                gen.x_dir = true;
                if (x_dist < 0) gen.x_dir = false;
                gen.y_step_count = (long)(Math.abs(y_dist) * (float)step_scale[1]);
                gen.y_total_step_count = gen.y_step_count;
                gen.y_dir = true;
                if (y_dist < 0) gen.y_dir = false;
                gen.z_step_count = (long)(Math.abs(z_dist) * (float)step_scale[2]);
                gen.z_total_step_count = gen.z_step_count;
                gen.z_dir = true;
                if (z_dist < 0) gen.z_dir = false;
                gen.x_delay = feedrates[0];
                gen.y_delay = feedrates[1];
                gen.z_delay = feedrates[2];
                pushMoveToStack(gen);
            }
            if (moves.get(x).Gword == 1)
            {
                float x_dist = (moves.get(x).Xword - move_dro_position[0]);
                float y_dist = (moves.get(x).Yword - move_dro_position[1]);
                float z_dist = (moves.get(x).Zword - move_dro_position[2]);
                //System.out.println("x_dist: " + x_dist);
                //System.out.println("y_dist: " + y_dist);
                long[] feedrates = getIndividualAxisFeedrates(moves.get(x).Fword, x_dist, y_dist, z_dist);
                StepGenStruct gen = new StepGenStruct();
                gen.x_step_count = (long)(Math.abs(x_dist) * (float)step_scale[0]);
                gen.x_total_step_count = gen.x_step_count;
                gen.x_dir = true;
                if (x_dist < 0) gen.x_dir = false;
                gen.y_step_count = (long)(Math.abs(y_dist) * (float)step_scale[1]);
                gen.y_total_step_count = gen.y_step_count;
                gen.y_dir = true;
                if (y_dist < 0) gen.y_dir = false;
                gen.z_step_count = (long)(Math.abs(z_dist) * (float)step_scale[2]);
                gen.z_total_step_count = gen.z_step_count;
                gen.z_dir = true;
                if (z_dist < 0) gen.z_dir = false;
                gen.x_delay = feedrates[0];
                gen.y_delay = feedrates[1];
                gen.z_delay = feedrates[2];
                pushMoveToStack(gen);
            }
            if (moves.get(x).Gword == 2)
            {

            }
            if (moves.get(x).Gword == 3)
            {

            }
            move_dro_position[0] = moves.get(x).Xword;
            move_dro_position[1] = moves.get(x).Yword;
            move_dro_position[2] = moves.get(x).Zword;
        }
    }
    public void next_move()
    {
        ArrayList<StepGenStruct> tmp_buffer = new ArrayList();
        if (move_buffer.size() > 0)
        {
            for (int x = 1; x < move_buffer.size(); x++)
            {
                tmp_buffer.add(move_buffer.get(x));
            }
        }
        move_buffer = tmp_buffer;
    }
    private int getDirectionFactor(int axis, StepGenStruct s)
    {
        if (axis == 0)
        {
            if (s.x_dir == true)
            {
                return 1;
            }
            else
            {
                return -1;
            }
        }
        if (axis == 1)
        {
            if (s.y_dir == true)
            {
                return 1;
            }
            else
            {
                return -1;
            }
        }
        if (axis == 2)
        {
            if (s.z_dir == true)
            {
                return 1;
            }
            else
            {
                return -1;
            }
        }
        return 1;
    }
    public void Poll()
    {

        /* we need to mimick the stepgen stack in order to simulate DRO output in real time, this way we won't tax the embedded controller */
        if (move_buffer.size() > 0) //There are moves on the stack!
        {
            //System.out.println("Moves on stack!");
            if (move_buffer.get(0).x_step_count == 0 && move_buffer.get(0).y_step_count == 0 && move_buffer.get(0).z_step_count == 0) //no more moves left, move on to next move
            {
                System.out.println("Next move");
                next_move();
                motion_dro_before_move = new float[] {GlobalData.dro[0], GlobalData.dro[1], GlobalData.dro[2]};
            }
            else
            {
                if (move_buffer.get(0).x_step_count > 0 && x_timer < micros()) //We know that are timer event has expired and surpassed many events, so we need to figure out how many events and subtract that from step_count
                {
                    long elapsed_time = micros() - x_timer;
                    if (elapsed_time < (1000 * 1000))
                    {
                        long number_of_steps = elapsed_time / move_buffer.get(0).x_delay;
                        move_buffer.get(0).x_step_count -= number_of_steps;
                        if (move_buffer.get(0).x_step_count < 0)
                        {
                            GlobalData.dro[0] = motion_dro_before_move[0] + ((1.0f / (float)step_scale[0]) * (float)Math.abs(move_buffer.get(0).x_total_step_count) * (getDirectionFactor(0, move_buffer.get(0))));
                            move_buffer.get(0).x_step_count = 0;
                        }
                        else
                        {
                            //System.out.println("X_STEP_COUNT: " + move_buffer.get(0).x_step_count + " Elapsed time: " + elapsed_time + " Incrementing steps: " + number_of_steps + " DRO_X_Increment: " + (1.0f / (float)step_scale[0]) * (float)number_of_steps);
                            GlobalData.dro[0] += (1.0f / (float)step_scale[0]) * (float)number_of_steps * getDirectionFactor(0, move_buffer.get(0)); //Increment dro!
                            x_timer = micros() + ((move_buffer.get(0).x_delay + step_len) * number_of_steps);
                        }

                    }
                    else
                    {
                        x_timer = micros();
                    }
                }
                if (move_buffer.get(0).y_step_count > 0 && y_timer < micros()) //We know that are timer event has expired and surpassed many events, so we need to figure out how many events and subtract that from step_count
                {
                    long elapsed_time = micros() - y_timer;
                    if (elapsed_time < (1000 * 1000))
                    {
                        long number_of_steps = elapsed_time / move_buffer.get(0).y_delay;
                        move_buffer.get(0).y_step_count -= number_of_steps;
                        if (move_buffer.get(0).y_step_count < 0)
                        {
                            GlobalData.dro[1] = motion_dro_before_move[1] + ((1.0f / (float)step_scale[1]) * (float)Math.abs(move_buffer.get(0).y_total_step_count) * (getDirectionFactor(1, move_buffer.get(0))));
                            move_buffer.get(0).y_step_count = 0;
                        }
                        else
                        {
                            //System.out.println("Y_STEP_COUNT: " + move_buffer.get(0).y_step_count + " Elapsed time: " + elapsed_time + " Incrementing steps: " + number_of_steps + " DRO_Y_Increment: " + (1.0f / (float)step_scale[1]) * (float)number_of_steps);
                            GlobalData.dro[1] += (1.0f / (float)step_scale[1]) * (float)number_of_steps * getDirectionFactor(1, move_buffer.get(0)); //Increment dro!
                            y_timer = micros() + ((move_buffer.get(0).y_delay + (step_len)) * number_of_steps);
                        }

                    }
                    else
                    {
                        y_timer = micros();
                    }
                }
                if (move_buffer.get(0).z_step_count > 0 && z_timer < micros()) //We know that are timer event has expired and surpassed many events, so we need to figure out how many events and subtract that from step_count
                {
                    long elapsed_time = micros() - z_timer;
                    if (elapsed_time < (1000 * 1000))
                    {
                        long number_of_steps = elapsed_time / move_buffer.get(0).z_delay;
                        move_buffer.get(0).z_step_count -= number_of_steps;
                        if (move_buffer.get(0).z_step_count < 0)
                        {
                            GlobalData.dro[2] = motion_dro_before_move[2] + ((1.0f / (float)step_scale[2]) * (float)Math.abs(move_buffer.get(0).z_total_step_count) * (getDirectionFactor(2, move_buffer.get(0))));
                            move_buffer.get(0).z_step_count = 0;
                        }
                        else
                        {
                            //System.out.println("Z_STEP_COUNT: " + move_buffer.get(0).z_step_count + " Elapsed time: " + elapsed_time + " Incrementing steps: " + number_of_steps + " DRO_Y_Increment: " + (1.0f / (float)step_scale[2]) * (float)number_of_steps);
                            GlobalData.dro[2] += (1.0f / (float)step_scale[2]) * (float)number_of_steps * getDirectionFactor(2, move_buffer.get(0)); //Increment dro!
                            z_timer = micros() + ((move_buffer.get(0).z_delay + (step_len)) * number_of_steps);
                        }
                    }
                    else
                    {
                        z_timer = micros();
                    }
                }
            }
        }
        else
        {
            /*if (x_timer == 0)
            {
                x_timer = micros();
            }
            if (y_timer == 0)
            {
                y_timer = micros();
            }
            if (z_timer == 0)
            {
                z_timer = micros();
            }*/
            motion_dro_before_move = new float[] {GlobalData.dro[0], GlobalData.dro[1], GlobalData.dro[2]};
        }
    }
}
