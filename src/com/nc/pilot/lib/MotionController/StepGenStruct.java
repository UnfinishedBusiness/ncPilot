package com.nc.pilot.lib.MotionController;

/**
 * Created by travi on 2/22/2019.
 */
public class StepGenStruct {
    long x_total_step_count;
    long y_total_step_count;
    long z_total_step_count;
    long x_step_count;
    long y_step_count;
    long z_step_count;
    long x_delay;
    long y_delay;
    long z_delay;
    boolean x_dir;
    boolean y_dir;
    boolean z_dir;

    float target_velocity;
    float current_velocity;
}
