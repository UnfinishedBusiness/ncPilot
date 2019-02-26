package com.nc.pilot.lib.MotionController;

import com.nc.pilot.lib.GcodeInterpreter;
import com.nc.pilot.lib.GlobalData;

import java.util.ArrayList;

/**
 * Created by travi on 2/22/2019.
 */
public class MotionEngine {

    private ArrayList<GcodeInterpreter.GcodeMove> moves;
    private ArrayList<StepGenStruct> move_buffer_without_planning;
    private ArrayList<StepGenStruct> move_buffer;
    private int current_move;
    private float[] move_dro_position;
    private float[] motion_dro_before_move;

    private static int step_len = 10; //this has to match what the step gen is set too!

    private static int motion_buffer_size = 10; //This has to match what is in the step gen controller!
    private static int motion_buffer_moves_ahead = 0; //We have to make sure we don't exceed $motion_buffer_size ahead
    private static int motion_instruction_max_step_count = 30000;

    private static long one_minute = 60000000; //60,000,000
    private static long x_timer;
    private static long y_timer;
    private static long z_timer;
    private static long motion_buffer_write_timer;

    private long[] step_scale;
    private float[] axis_acceleration; //Inch/Sec^2
    private float max_linear_velocity = 800;

    public MotionEngine(){
        move_buffer = new ArrayList();
        move_buffer_without_planning = new ArrayList();
        step_scale = new long[] {650, 650, 650};
        axis_acceleration = new float[] {10f, 10f, 10f};

        x_timer = micros();
        y_timer = micros();
        z_timer = micros();

        GlobalData.dro[0] = 0;
        GlobalData.dro[1] = 0;
        GlobalData.dro[2] = 0;

        current_move = 0;
    }
    private long micros()
    {
        return System.nanoTime() / 500;
    }
    private static boolean inTolerance(float a, float b, float t)
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
    public static float getAngle(float[] start_point, float[] end_point) {
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
    public static float getLineLength(float[] start_point, float[] end_point)
    {
        return new Float(Math.hypot(start_point[0]-end_point[0], start_point[1]-end_point[1]));
    }
    public static float[] rotatePoint(float[] pivot, float[] rotated_point, float angle)
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
    public static float[] getPolarLineEndpoint(float[] start_point, float length, float angle)
    {
        float[] end_point = new float[] {start_point[0] + length, start_point[1]};
        return rotatePoint(start_point, end_point, angle);
    }
    public static ArrayList<float[]> getPointsOfArc(float[] start, float[] end, float[] center, float radius, String direction, int angle_inc)
    {
        float start_angle = getAngle(center, start);
        float end_angle = getAngle(center, end);
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
            //float [] new_point = getPolarLineEndpoint(center, radius, end_angle);
            //points.add(new_point);
            points.add(end);
        }
        return points;
    }
    public float distanceBetween3DPoints(float[] p1, float[] p2)
    {
        return (float) Math.sqrt(Math.pow(p1[0] - p2[0], 2) + Math.pow(p1[1] - p2[1], 2) + Math.pow(p1[2] - p2[2], 2));
    }
    private long[] getIndividualAxisFeedrates(float f, float x_dist, float y_dist, float z_dist, float cartesion_dist)
    {
        long x_feed = 0;
        long y_feed = 0;
        long z_feed = 0;
        //System.out.println("Cartesion Distance: " + cartesion_dist);
        long move_will_take = (long) ((cartesion_dist / f) * one_minute); //Move will take 'x' nanoseconds to finish at specified feedrate
        x_feed = (long)(move_will_take / ((float)step_scale[0] * Math.abs(x_dist)));
        y_feed = (long)(move_will_take / ((float)step_scale[1] * Math.abs(y_dist)));
        z_feed = (long)(move_will_take / ((float)step_scale[2] * Math.abs(z_dist)));

        if (x_feed > 9000000000000000000l) x_feed = 0;
        if (y_feed > 9000000000000000000l) y_feed = 0;
        if (z_feed > 9000000000000000000l) z_feed = 0;
        return new long[] {x_feed, y_feed, z_feed};
    }
    float map(float x, float in_min, float in_max, float out_min, float out_max)
    {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
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
    private void pushLinearMove_NonPlanned(float[] start_point, float[] end_point, float feedrate)
    {
        float x_dist = (end_point[0] - start_point[0]);
        float y_dist = (end_point[1] - start_point[1]);
        float z_dist = (end_point[2] - start_point[2]);
        //System.out.println("x_dist: " + x_dist);
        //System.out.println("y_dist: " + y_dist);
        long[] feedrates = getIndividualAxisFeedrates(feedrate, x_dist, y_dist, z_dist, distanceBetween3DPoints(start_point, end_point));
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
        gen.target_velocity = feedrate;
        gen.current_velocity = 0; //planMotionBuffer will increment these values
        gen.vector_angle = getAngle(start_point, end_point);
        gen.start_point = start_point;
        gen.end_point = end_point;
        move_buffer_without_planning.add(gen);
    }
    private void pushLinearMove(float[] start_point, float[] end_point, float feedrate)
    {
        float x_dist = (end_point[0] - start_point[0]);
        float y_dist = (end_point[1] - start_point[1]);
        float z_dist = (end_point[2] - start_point[2]);
        //System.out.println("x_dist: " + x_dist);
        //System.out.println("y_dist: " + y_dist);
        long[] feedrates = getIndividualAxisFeedrates(feedrate, x_dist, y_dist, z_dist, distanceBetween3DPoints(start_point, end_point));
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
        gen.target_velocity = feedrate;
        gen.current_velocity = 0; //planMotionBuffer will increment these values
        gen.vector_angle = getAngle(start_point, end_point);
        gen.start_point = start_point;
        gen.end_point = end_point;
        move_buffer.add(gen);
    }
    private void planMotionBuffer()
    {
        System.out.println("planMotionBuffer()");
        // This method need to iterate the motion_buffer_without_acceleration and add in acceleration and decelleration planning
        float current_velocity = 1;
        for (int x = 0; x < move_buffer_without_planning.size(); x++)
        {
            if (move_buffer_without_planning.get(x).z_step_count == 0) //We are an XY move
            {
                System.out.println("Moving from (" + move_buffer_without_planning.get(x).start_point[0] + ", " + move_buffer_without_planning.get(x).start_point[1] + ") to (" + move_buffer_without_planning.get(x).end_point[0] + ", " + move_buffer_without_planning.get(x).end_point[1] + ")");
                System.out.println("Iterating across move -> " + x);
                float major_moves_polar_angle = getAngle(move_buffer_without_planning.get(x).start_point, move_buffer_without_planning.get(x).end_point);
                System.out.println("Polar Angle: " + major_moves_polar_angle);
                float major_move_distance = distanceBetween3DPoints(move_buffer_without_planning.get(x).start_point, move_buffer_without_planning.get(x).end_point);
                float velocity_update_distance = 0.1f; //Update velocity every x distance of travel
                if (move_buffer_without_planning.size() > x+1) //There is another move
                {
                    float current_length = 0;
                    float[] last_endpoint = move_buffer_without_planning.get(x).start_point;
                    while(current_length < major_move_distance && current_velocity < move_buffer_without_planning.get(x).target_velocity)
                    {
                        float velocity_change_over_velocity_update_distance = (axis_acceleration[0] * current_velocity) * velocity_update_distance;
                        current_velocity += velocity_change_over_velocity_update_distance;
                        current_length += velocity_update_distance;
                        System.out.println("Current Velocity: " + current_velocity + " Target Velocity: " + move_buffer_without_planning.get(x).target_velocity);
                        float[] end_point = getPolarLineEndpoint(last_endpoint, current_length, major_moves_polar_angle);
                        pushLinearMove(new float[] {last_endpoint[0], last_endpoint[1], 0}, new float[] {end_point[0], end_point[1], 0}, current_velocity);
                        last_endpoint = end_point;
                    }
                    if (current_length < major_move_distance) //We accelerated to target velocity before major move ended so finish move at target velocity
                    {
                        current_length = major_move_distance;
                        float[] end_point = getPolarLineEndpoint(last_endpoint, current_length, major_moves_polar_angle);
                        pushLinearMove(new float[] {last_endpoint[0], last_endpoint[1], 0}, new float[] {end_point[0], end_point[1], 0}, current_velocity);
                    }

                }
                else
                {
                    //plan a complete stop
                }
            }
            else
            {
                move_buffer.add(move_buffer_without_planning.get(x));
            }
        }
    }
    public void runMoves()
    {
        //System.out.println("runMoves()");
        move_dro_position = new float[] {GlobalData.dro[0], GlobalData.dro[1], GlobalData.dro[2]};
        for (int x = 0; x < moves.size(); x++)
        {
            if (moves.get(x).Gword == 0)
            {
                pushLinearMove_NonPlanned(move_dro_position, new float[] {moves.get(x).Xword, moves.get(x).Yword, moves.get(x).Zword}, max_linear_velocity);
            }
            if (moves.get(x).Gword == 1)
            {
                pushLinearMove_NonPlanned(move_dro_position, new float[] {moves.get(x).Xword, moves.get(x).Yword, moves.get(x).Zword}, moves.get(x).Fword);
            }
            if (moves.get(x).Gword == 2)
            {
                float[] center = new float[]{move_dro_position[0] + moves.get(x).Iword, move_dro_position[1] + moves.get(x).Jword};
                float radius = new Float(Math.hypot(moves.get(x).Xword-center[0], moves.get(x).Yword-center[1]));
                ArrayList<float[]> points = getPointsOfArc(move_dro_position, new float[] {moves.get(x).Xword, moves.get(x).Yword }, center, radius, "CW", 10);
                for (int y = 1; y < points.size(); y++)
                {
                    pushLinearMove_NonPlanned(new float[] {points.get(y-1)[0], points.get(y-1)[1], 0 }, new float[] {points.get(y)[0], points.get(y)[1], 0 }, moves.get(x).Fword);
                }
            }
            if (moves.get(x).Gword == 3)
            {
                float[] center = new float[]{move_dro_position[0] + moves.get(x).Iword, move_dro_position[1] + moves.get(x).Jword};
                float radius = new Float(Math.hypot(moves.get(x).Xword-center[0], moves.get(x).Yword-center[1]));
                ArrayList<float[]> points = getPointsOfArc(move_dro_position, new float[] {moves.get(x).Xword, moves.get(x).Yword }, center, radius, "CCW", 10);
                for (int y = 1; y < points.size(); y++)
                {
                    pushLinearMove_NonPlanned(new float[] {points.get(y-1)[0], points.get(y-1)[1], 0 }, new float[] {points.get(y)[0], points.get(y)[1], 0 }, moves.get(x).Fword);
                }
            }
            move_dro_position[0] = moves.get(x).Xword;
            move_dro_position[1] = moves.get(x).Yword;
            move_dro_position[2] = moves.get(x).Zword;


            /*if (x == 0) //If we are the first move, push it to the StepGet Motion Stack as well
            {
                if (move_buffer.size() > 0)
                {
                    StepGenStruct a = adjustStepsForAcuracyDeviation(move_buffer.get(0));
                    String send_buf = ">" + getStepCountWithDirection(0, a) + ":" + a.x_delay + "|" +  getStepCountWithDirection(1, a) + ":" + a.y_delay + "|" + getStepCountWithDirection(2, a) + ":" + a.z_delay + "\n";
                    System.out.print(send_buf);
                    MotionController.WriteBuffer(send_buf);
                }
            }*/
        }
        planMotionBuffer();
    }
    public void next_move()
    {
        /*motion_buffer_moves_ahead--;
        if (motion_buffer_moves_ahead < 0) motion_buffer_moves_ahead = 0;
        ArrayList<StepGenStruct> tmp_buffer = new ArrayList();
        if (move_buffer.size() > 0)
        {
            for (int x = 1; x < move_buffer.size(); x++)
            {
                tmp_buffer.add(move_buffer.get(x));
            }
        }
        move_buffer = tmp_buffer;*/
        motion_buffer_moves_ahead--;
        current_move++;
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
    public void FillMotionBufferOnStepgen()
    {
        //Keep sending move to stepgen until we fill up the buffer
        if (motion_buffer_write_timer < micros())
        {
            //System.out.println("Motion Buffer is " + motion_buffer_moves_ahead + " moves ahead of current move!");
            motion_buffer_write_timer = micros() + (50 * 1000);
            if (motion_buffer_moves_ahead < motion_buffer_size && (motion_buffer_moves_ahead + current_move + 1) < move_buffer.size()) //There are moves left to push onto stepgen stack
            {
                motion_buffer_moves_ahead++;
                StepGenStruct a = adjustStepsForAcuracyDeviation(move_buffer.get(current_move + motion_buffer_moves_ahead));
                String send_buf = ">" + getStepCountWithDirection(0, a) + ":" + a.x_delay + "|" +  getStepCountWithDirection(1, a) + ":" + a.y_delay + "|" + getStepCountWithDirection(2, a) + ":" + a.z_delay + "\n";
                System.out.print(send_buf);
                MotionController.WriteBuffer(send_buf);
            }
        }
    }
    public void Poll()
    {

        /* we need to mimick the stepgen stack in order to simulate DRO output in real time, this way we won't tax the embedded controller */
        if (current_move < move_buffer.size()) //There are moves on the stack!
        {
            //System.out.println("Moves on stack!");
            if (move_buffer.get(current_move).x_step_count == 0 && move_buffer.get(current_move).y_step_count == 0 && move_buffer.get(current_move).z_step_count == 0) //no more moves left, move on to next move
            {
                System.out.println("Next move");
                next_move();
                if (current_move < move_buffer.size())
                {
                    motion_dro_before_move = new float[]{GlobalData.dro[0], GlobalData.dro[1], GlobalData.dro[2]};
                    GlobalData.ProgrammedFeedrate = move_buffer.get(current_move).target_velocity;
                }
            }
            else
            {
                FillMotionBufferOnStepgen();
                if (move_buffer.get(current_move).x_step_count > 0 && x_timer < micros()) //We know that are timer event has expired and surpassed many events, so we need to figure out how many events and subtract that from step_count
                {
                    long elapsed_time = micros() - x_timer;
                    if (elapsed_time < (100 * 1000))
                    {
                        long number_of_steps = elapsed_time / move_buffer.get(current_move).x_delay;
                        if (number_of_steps == 0) number_of_steps++;
                        move_buffer.get(current_move).x_step_count -= number_of_steps;
                        if (move_buffer.get(current_move).x_step_count < 0)
                        {
                            GlobalData.dro[0] = motion_dro_before_move[0] + ((1.0f / (float)step_scale[0]) * (float)Math.abs(move_buffer.get(current_move).x_total_step_count) * (getDirectionFactor(0, move_buffer.get(current_move))));
                            move_buffer.get(current_move).x_step_count = 0;
                        }
                        else
                        {
                            //System.out.println("X_STEP_COUNT: " + move_buffer.get(0).x_step_count + " Elapsed time: " + elapsed_time + " Incrementing steps: " + number_of_steps + " DRO_X_Increment: " + (1.0f / (float)step_scale[0]) * (float)number_of_steps);
                            GlobalData.dro[0] += (1.0f / (float)step_scale[0]) * (float)number_of_steps * getDirectionFactor(0, move_buffer.get(current_move)); //Increment dro!
                            x_timer = micros() + ((move_buffer.get(current_move).x_delay + step_len) * number_of_steps);
                        }

                    }
                    else
                    {
                        x_timer = micros();
                    }
                }
                if (move_buffer.get(current_move).y_step_count > 0 && y_timer < micros()) //We know that are timer event has expired and surpassed many events, so we need to figure out how many events and subtract that from step_count
                {
                    long elapsed_time = micros() - y_timer;
                    if (elapsed_time < (100 * 1000))
                    {
                        long number_of_steps = elapsed_time / move_buffer.get(current_move).y_delay;
                        if (number_of_steps == 0) number_of_steps++;
                        move_buffer.get(current_move).y_step_count -= number_of_steps;
                        if (move_buffer.get(current_move).y_step_count < 0)
                        {
                            GlobalData.dro[1] = motion_dro_before_move[1] + ((1.0f / (float)step_scale[1]) * (float)Math.abs(move_buffer.get(current_move).y_total_step_count) * (getDirectionFactor(1, move_buffer.get(current_move))));
                            move_buffer.get(current_move).y_step_count = 0;
                        }
                        else
                        {
                            //System.out.println("Y_STEP_COUNT: " + move_buffer.get(0).y_step_count + " Elapsed time: " + elapsed_time + " Incrementing steps: " + number_of_steps + " DRO_Y_Increment: " + (1.0f / (float)step_scale[1]) * (float)number_of_steps);
                            GlobalData.dro[1] += (1.0f / (float)step_scale[1]) * (float)number_of_steps * getDirectionFactor(1, move_buffer.get(current_move)); //Increment dro!
                            y_timer = micros() + ((move_buffer.get(current_move).y_delay + (step_len)) * number_of_steps);
                        }

                    }
                    else
                    {
                        y_timer = micros();
                    }
                }
                if (move_buffer.get(current_move).z_step_count > 0 && z_timer < micros()) //We know that are timer event has expired and surpassed many events, so we need to figure out how many events and subtract that from step_count
                {
                    long elapsed_time = micros() - z_timer;
                    if (elapsed_time < (100 * 1000))
                    {
                        long number_of_steps = elapsed_time / move_buffer.get(current_move).z_delay;
                        if (number_of_steps == 0) number_of_steps++;
                        //System.out.println("Z_DELAY: " + move_buffer.get(0).z_delay);
                        move_buffer.get(current_move).z_step_count -= number_of_steps;
                        if (move_buffer.get(current_move).z_step_count < 0)
                        {
                            GlobalData.dro[2] = motion_dro_before_move[2] + ((1.0f / (float)step_scale[2]) * (float)Math.abs(move_buffer.get(current_move).z_total_step_count) * (getDirectionFactor(2, move_buffer.get(current_move))));
                            move_buffer.get(current_move).z_step_count = 0;
                        }
                        else
                        {
                            //System.out.println("Z_STEP_COUNT: " + move_buffer.get(0).z_step_count + " Elapsed time: " + elapsed_time + " Incrementing steps: " + number_of_steps + " DRO_Z_Increment: " + (1.0f / (float)step_scale[2]) * (float)number_of_steps);
                            GlobalData.dro[2] += (1.0f / (float)step_scale[2]) * (float)number_of_steps * getDirectionFactor(2, move_buffer.get(current_move)); //Increment dro!
                            z_timer = micros() + ((move_buffer.get(current_move).z_delay + (step_len)) * number_of_steps);
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
            if (x_timer == 0)
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
            }
            motion_dro_before_move = new float[] {GlobalData.dro[0], GlobalData.dro[1], GlobalData.dro[2]};
        }
    }
}
