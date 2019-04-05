package com.nc.pilot.ui;

import com.nc.pilot.dialogs.JetToolpathCutChart;
import com.nc.pilot.dialogs.JetToolpathJobSetup;
import com.nc.pilot.lib.GlobalData;
import com.nc.pilot.lib.ToolPathViewer.ToolpathViewer;
import com.nc.pilot.lib.UIWidgets.UIWidgets;
import org.kabeja.parser.ParseException;

import javax.swing.*;
import javax.swing.Timer;
import javax.swing.filechooser.FileFilter;
import java.awt.*;
import java.awt.event.*;
import java.io.File;
import java.io.FileNotFoundException;
import java.util.*;

public class MotionSimulator extends JFrame {
    JMenuBar menu_bar;
    UIWidgets ui_widgets;

    java.util.Timer interupt_timer = new java.util.Timer();
    java.util.Timer repaint_timer = new java.util.Timer();

    /* Machine Parameters */
    float min_feed_rate = 0.25f;
    int x_scale = 400; //Steps per inch
    int y_scale = 325;
    float x_accel = 5.0f;
    float max_linear_velocity = 25.0f;
    /* ---------- */

    /* These units are in steps */
    int[] target_position;
    int[] machine_position;
    /* --------- */

    /* These are in scaled units */
    float[] last_position = {0.0f, 0.0f};
    float[] machine_position_dro;
    float[] target_position_in_real_units;
    /* ------------ */

    int dx, dy, err, e2, sx, sy, x0, x1, y0, y1;
    long velocity_update_timestamp;
    int sample_period = 5; //Sample every x milliseconds
    float x_scale_inverse = 1 / (float)x_scale;
    float y_scale_inverse = 1 / (float)y_scale;

    /* Variables updated by sample check and used my move */
    float target_velocity = 0.0f;
    float linear_velocity = 0.0f;
    float x_velocity = 0.0f;
    float y_velocity = 0.0f;
    long motion_timestamp = 0;
    long move_start_timestamp = 0;
    long move_decel_timestamp = 0;
    boolean InMotion = false;
    long x_dist_in_steps;
    long y_dist_in_steps;
    float decceleration_dtg_marker;
    long time_required_to_accelerate;
    /* ---------------- */

    int cycle_speed = 0;

    float filter(float rawValue, float weight, float lastValue) {
        // run the filter:
        float result = (float) (weight * rawValue + (1.0-weight)*lastValue);
        // return the result:
        return result;
    }
    public float getDistance(float[] start_point, float[] end_point)
    {
        return new Float(Math.hypot(start_point[0]-end_point[0], start_point[1]-end_point[1]));
    }
    //Initial Velocity is in units per minute, acceleration is in units/min^2, time is in milliseconds
    public float getAcceleratedVelocity(float initial_velocity, float acceleration, float time_into_move) //Returns feedrate in inches/min
    {
       return (initial_velocity / 60.0f) + ((acceleration / 60.0f) * (time_into_move / 1000.0f)) * 60.0f;
    }
    public void setFeedrate(float feedrate)
    {
        linear_velocity = feedrate;
        System.out.println("Setting Feedrate too: " + feedrate);
        x_dist_in_steps = Math.abs(target_position[0] - machine_position[0]);
        y_dist_in_steps = Math.abs(target_position[1] - machine_position[1]);
        float  linear_distance_in_scaled_units = getDistance(machine_position_dro, target_position_in_real_units);
        if (x_dist_in_steps > y_dist_in_steps) //The x axis has farther to travel. Coordinate feedrate on X axis
        {
            cycle_speed = (int)(((linear_distance_in_scaled_units / feedrate) * 60000.0f) / x_dist_in_steps) - 1;
        }
        else
        {
            cycle_speed = (int)(((linear_distance_in_scaled_units / feedrate) * 60000.0f) / y_dist_in_steps) - 1;
        }
    }
    void set_target_position(float x, float y, float target_feed_rate)
    {
        System.out.println("Target velocity - " + target_feed_rate);
        target_position_in_real_units = new float[] {x, y};
        float total_move_distance = getDistance(machine_position_dro, target_position_in_real_units);
        System.out.println("total_move_distance - " + total_move_distance);
        last_position = new float[] {machine_position_dro[0], machine_position_dro[1]};
        target_position = new int[]{(int)(x * x_scale), (int)(y * y_scale)};
        move_decel_timestamp = 0;
        time_required_to_accelerate = (long) ((((target_feed_rate / 60.0f) - (min_feed_rate / 60.0f)) / (x_accel / 60.0f)) * 60.0f);
        System.out.println("time_required_to_accelerate - " + time_required_to_accelerate);
        float distance_required_to_accelerate = (float) (((min_feed_rate / 60.0f) * (time_required_to_accelerate / 60.0f)) + 0.5f * (x_accel / 60.0f) * Math.pow(time_required_to_accelerate / 60.0f, 2));
        System.out.println("distance_required_to_accelerate - " + distance_required_to_accelerate);
        //Can we accelerate to the target velocity and decelerate to exit velocity in the distance we have to travel?
        if (distance_required_to_accelerate * 2 < total_move_distance)
        {
            System.out.println("We can accelerate to target velocity!");
            target_velocity = target_feed_rate;
            //Set the deceleration marker. We need to start decelerating when we get to "total_move_distance - distance_required_to_accelerate" on distance traveled
            decceleration_dtg_marker = total_move_distance - distance_required_to_accelerate;
            System.out.println("decceleration_dtg_marker - " + decceleration_dtg_marker);
        }
        else
        {
            System.out.println("We can't accelerate to target velocity!");
            target_velocity = target_feed_rate;
            //Figure out the maximum feedrate from acceleration on half of the distance to travel
        }
        setFeedrate(min_feed_rate); //Set initial feedrate for move

        x1 = target_position[0];
        y1 = target_position[1];
        x0 = machine_position[0];
        y0 = machine_position[1];
        dx = Math.abs(x1-x0);
        sx = x0<x1 ? 1 : -1;
        dy = Math.abs(y1-y0);
        sy = y0<y1 ? 1 : -1;
        err = (dx>dy ? dx : -dy)/2;
        move_start_timestamp = System.currentTimeMillis();
    }

    private void interupt()
    {
        if ((System.currentTimeMillis() - velocity_update_timestamp) > sample_period && InMotion == true)
        {
            float sample_distance = getDistance(machine_position_dro, last_position);
            //System.out.println("Sample Distance: " + sample_distance);
            //(distance/time)*60,000 = velocity in steps per minute
            //linear_velocity = (sample_distance/(System.currentTimeMillis() - move_start_timestamp))*60000;
            x_velocity = (Math.abs(machine_position_dro[0] - last_position[0])/(System.currentTimeMillis() - move_start_timestamp))*60000; //in steps per minute
            y_velocity = (Math.abs(machine_position_dro[1] - last_position[1])/(System.currentTimeMillis() - move_start_timestamp))*60000; //in steps per minute
            float distance_traveled = getDistance(machine_position_dro, last_position);
            float new_velocity = getAcceleratedVelocity(min_feed_rate, x_accel, (System.currentTimeMillis() - move_start_timestamp));
            //System.out.println("new_velocity - " + new_velocity);
            if (distance_traveled < decceleration_dtg_marker)
            {
                //We accelerate to target velocity
                if (new_velocity < target_velocity)
                {
                    setFeedrate(new_velocity);
                }
                else
                {
                    setFeedrate(target_velocity);
                }
                move_decel_timestamp = 0;
            }
            else if (move_decel_timestamp == 0)
            {
                move_decel_timestamp = System.currentTimeMillis();
                //System.out.println("move_decel_timestamp - " + move_decel_timestamp);
            }
            else if (move_decel_timestamp == -1)
            {

            }
            else
            {
                //We deccelerate to target velocity
                //System.out.println("Decelleration Period: " + (time_required_to_accelerate - (System.currentTimeMillis() - move_decel_timestamp)));
                new_velocity = getAcceleratedVelocity(target_velocity, x_accel, time_required_to_accelerate - (System.currentTimeMillis() - move_decel_timestamp));
                //System.out.println("new_velocity - " + new_velocity);
                if (new_velocity < min_feed_rate)
                {
                    //System.out.println("Clamping feedrate to min feed rate to ensure move finishes!");
                    setFeedrate(min_feed_rate);
                    move_decel_timestamp = -1; //Deceleration is finished
                }
                else
                {
                    setFeedrate(new_velocity);
                }
            }

            velocity_update_timestamp = System.currentTimeMillis();
        }
        if ((System.currentTimeMillis() - motion_timestamp) > cycle_speed)
        {
            machine_position[0] = x0;
            machine_position[1] = y0;
            machine_position_dro[0] = machine_position[0] * x_scale_inverse;
            machine_position_dro[1] = machine_position[1] * y_scale_inverse;
            if (x0==x1 && y0==y1)
            {
                //We are at our target position. Set next target position
                InMotion = false;
            }
            else
            {
                InMotion = true;
                e2 = err;
                if (e2 >-dx) { err -= dy; x0 += sx; }
                if (e2 < dy) { err += dx; y0 += sy; }
            }
            motion_timestamp = System.currentTimeMillis();
        }
    }

    public MotionSimulator() {

        super("Motion Simulator");
        //GlobalData.configData.CurrentWorkbench = "MotionSimulator";
        setSize(1100, 800);
        setExtendedState(JFrame.MAXIMIZED_BOTH);
        setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
        setLocationRelativeTo(null);

        ui_widgets = new UIWidgets();
        
        Layout_UI();
        createMenuBar();
        setJMenuBar(menu_bar);
        ToolpathViewerPanel panel = new ToolpathViewerPanel();
        add(panel);

        machine_position = new int[] {0, 0};
        machine_position_dro = new float[] { machine_position[0] * x_scale_inverse, machine_position[1] * y_scale_inverse};


        interupt_timer.schedule(new TimerTask() {
            @Override
            public void run() {
                interupt();
            }
        }, 0, 1);

        repaint_timer.schedule(new TimerTask() {
            @Override
            public void run() {
                repaint();
            }
        }, 0, 30);


        KeyboardFocusManager.getCurrentKeyboardFocusManager()
                .addKeyEventDispatcher(new KeyEventDispatcher() {
                    @Override
                    public boolean dispatchKeyEvent(KeyEvent ke) {
                        //if (!GlobalData.configData.CurrentWorkbench.contentEquals("MotionSimulator")) return false;
                        switch (ke.getID()) {
                            case KeyEvent.KEY_PRESSED:
                                //System.out.println("(Jet Toolpath) Key: " + ke.getKeyCode());
                                if (ke.getKeyCode() == 44) //< key
                                {

                                    repaint();
                                }
                                if (ke.getKeyCode() == 46) //> key
                                {

                                    repaint();
                                }

                                //repaint();
                                break;

                            case KeyEvent.KEY_RELEASED:
                                if (ke.getKeyCode() == KeyEvent.VK_SPACE) {
                                    set_target_position(2f, 1.5f, 10f);
                                    repaint();
                                }
                                if (ke.getKeyCode() == KeyEvent.VK_1) {
                                    set_target_position(1.0f, 0.250f, 20f);
                                    repaint();
                                }
                                if (ke.getKeyCode() == KeyEvent.VK_2) {
                                    set_target_position(1.0f, 1.0f, 30f);
                                    repaint();
                                }
                                break;
                        }
                        return false;
                    }
                });

        // call onCancel() when cross is clicked
        setDefaultCloseOperation(EXIT_ON_CLOSE);
        addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent e) {
                onClose();
            }
        });

    }
    private void onClose() {
        // add your code here if necessary
        //GlobalData.configData.CurrentWorkbench = "MachineControl";
        dispose();
    }
    private void createMenuBar()
    {
        //Where the GUI is created:
        JMenu menu;
        JMenuItem menuItem;

        //Create the menu bar.
        menu_bar = new JMenuBar();

        //Build File menu
        menu = new JMenu("File");
        menu.setMnemonic(KeyEvent.VK_S);
        menu.getAccessibleContext().setAccessibleDescription("File operations");
        menu_bar.add(menu);

        menuItem = new JMenuItem("Open Job");
        menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_1, ActionEvent.ALT_MASK));
        menuItem.getAccessibleContext().setAccessibleDescription("Open Job File");
        menuItem.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                JFileChooser fileChooser = new JFileChooser();
                fileChooser.setCurrentDirectory(new File("."));
                int result = fileChooser.showOpenDialog(getParent());
                if (result == JFileChooser.APPROVE_OPTION) {
                    File selectedFile = fileChooser.getSelectedFile();
                    System.out.println("Selected file: " + selectedFile.getAbsolutePath());
                    //selectedFile.getAbsolutePath();
                    repaint();
                }
            }
        });
        menu.add(menuItem);
    }
    private void Layout_UI()
    {

    }
    // create a panel that you can draw on.
    class ToolpathViewerPanel extends JPanel {
        public void paint(Graphics g) {
            Rectangle Frame_Bounds = this.getParent().getBounds();
            GlobalData.MousePositionX_MCS = (GlobalData.MousePositionX - GlobalData.ViewerPan[0]) / GlobalData.ViewerZoom;
            GlobalData.MousePositionY_MCS = ((GlobalData.MousePositionY - GlobalData.ViewerPan[1]) / GlobalData.ViewerZoom) * -1;
            Graphics2D g2d = (Graphics2D) g;
            /* Begin Wallpaper */
            g.setColor(Color.black);
            g.fillRect(0,0,Frame_Bounds.width,Frame_Bounds.height);
            /* End Wallpaper */
            g.setFont(new Font("Arial", Font.PLAIN, 40));
            g.setColor(Color.green);
            g.drawString("+", machine_position[0], machine_position[1]);
            ui_widgets.RenderStack(g2d, Frame_Bounds);
            //Display Mouse position in MCS and Screen Cord

            g.setColor(Color.green);
            g.setFont(new Font("Arial", Font.BOLD, 12));
            g.drawString(String.format("DRO MCS-> X: %.4f Y: %.4f", machine_position_dro[0], machine_position_dro[1]), 10, 10);
            g.drawString(String.format("Linear Velocity -> %.2f Inch/min", linear_velocity), 10, 25);
            g.drawString(String.format("X Velocity -> %.2f Inch/min", x_velocity), 10, 40);
            g.drawString(String.format("Y Velocity -> %.2f Inch/min", y_velocity), 10, 55);
            g.drawString(String.format("Target Linear Velocity -> %.2f Inch/min", target_velocity), 10, 70);
            g.drawString(String.format("Distance Traveled -> %.4f Inches", getDistance(machine_position_dro, last_position)), 10, 85);
        }
    }
    public static void main(String[] args) {
        SwingUtilities.invokeLater(new Runnable() {
            @Override
            public void run() {
                new MotionSimulator().setVisible(true);
            }
        });
    }
}