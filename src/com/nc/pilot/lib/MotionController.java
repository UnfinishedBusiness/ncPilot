/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.nc.pilot.lib;

/**
 *
 * @author travis
 */

/*
$0=10 (step pulse, usec)
$1=25 (step idle delay, msec)
$2=0 (step port invert mask:00000000)
$3=6 (dir port invert mask:00000110)
$4=0 (step enable invert, bool)
$5=0 (limit pins invert, bool)
$6=0 (probe pin invert, bool)
$10=3 (status report mask:00000011)
$11=0.020 (junction deviation, mm)
$12=0.002 (arc tolerance, mm)
$13=0 (report inches, bool)
$20=0 (soft limits, bool)
$21=0 (hard limits, bool)
$22=0 (homing cycle, bool)
$23=1 (homing dir invert mask:00000001)
$24=50.000 (homing feed, mm/min)
$25=635.000 (homing seek, mm/min)
$26=250 (homing debounce, msec)
$27=1.000 (homing pull-off, mm)
$100=314.961 (x, step/mm)
$101=314.961 (y, step/mm)
$102=314.961 (z, step/mm)
$110=635.000 (x max rate, mm/min)
$111=635.000 (y max rate, mm/min)
$112=635.000 (z max rate, mm/min)
$120=50.000 (x accel, mm/sec^2)
$121=50.000 (y accel, mm/sec^2)
$122=50.000 (z accel, mm/sec^2)
$130=225.000 (x max travel, mm)
$131=125.000 (y max travel, mm)
$132=170.000 (z max travel, mm)
*/
public class MotionController {

    /* End Default Parameters */
    private static SerialIO serial;
    public MotionController(SerialIO s)
    {
        serial = s;
    }

    public static String SatusReport = "?\n";
    public static String GoToOrigin = "G90 G0 X0 Y0\r\n";

    public static String Home = "G53\r\nG28.2 Y0\r\nG28.2 X0\r\n";

    public static String FeedHold = "!\r\n";
    public static String CycleStart = "~\r\n";
    public static String QueFlush = "$clear\r\n%\r\n";
    
    //Y Jogging
    public static String StartJogYPlus = "G53 G0 Y" + ncConfig.YMaxTravel +"\r\n";
    public static String StopJogYPlus = "!%\r\nG54\r\n";
    
    public static String StartJogYMinus = "G53 G0 Y" + ncConfig.YMinTravel +"\r\n";
    public static String StopJogYMinus = "!%\r\nG54\r\n";
    
    //X Jogging
    public static String StartJogXPlus = "G53 G0 X" + ncConfig.XMaxTravel +"\r\n";
    public static String StopJogXPlus = "!%\r\nG54\r\n";
    
    public static String StartJogXMinus = "G53 G0 X" + ncConfig.XMinTravel +"\r\n";
    public static String StopJogXMinus = "!%\r\nG54\r\n";
    
    //Z Jogging
    public static String StartJogZPlus = "G53 G0 Z" + ncConfig.ZMaxTravel +"\r\n";
    public static String StopJogZPlus = "!%\r\nG54\r\n";
    
    public static String StartJogZMinus = "G53 G0 Z" + ncConfig.ZMinTravel +"\r\n";
    public static String StopJogZMinus = "!%\r\nG54\r\n";
    
    //Set origins
    public static String SetXOrigin = "G54 G92 X0.000\r\n";
    public static String SetYOrigin = "G54 G92 Y0.000\r\n";
    public static String SetZOrigin = "G92 Z=0\r\nG54\r\nG28.3 Z=0.000\r\n";
    
    //M Codes
    public static String TorchOn = "M3\r\n";
    public static String TorchOff = "M5\r\n";

    private static float jog_speed = 0;

    public static void WriteBuffer(String data){
        //GlobalData.WriteBuffer.add(data);
        serial.write(data);
    }
    public static void ReadBuffer(String inputLine){
        //System.out.println(inputLine);
        if (inputLine.equals("ok"))
        {
            //System.out.println("OK Recieved!");
                        /*if (GlobalData.WriteBuffer.size() > 0)
                        {
                            //System.out.println(GlobalData.WriteBuffer.size() + " Writing: " + GlobalData.WriteBuffer.get(0));
                            try {
                                Thread.sleep(100);
                            } catch (InterruptedException ex) {
                                Thread.currentThread().interrupt();
                            }
                            write(GlobalData.WriteBuffer.get(0));
                            ArrayList<String> TmpBuffer = new ArrayList();
                            for (int x = 1; x < GlobalData.WriteBuffer.size(); x++)
                            {
                                TmpBuffer.add(GlobalData.WriteBuffer.get(x));
                            }
                            GlobalData.WriteBuffer.clear();
                            for (int x = 0; x < TmpBuffer.size(); x++)
                            {
                                GlobalData.WriteBuffer.add(TmpBuffer.get(x));
                            }

                        }*/
        }
        else if (inputLine.contains("<") && inputLine.contains(">")) //Status Report
        {
            GlobalData.last_dro[0] = GlobalData.dro[0];
            GlobalData.last_dro[1] = GlobalData.dro[1];
            GlobalData.last_dro[2] = GlobalData.dro[2];
            String status_output = inputLine.substring(inputLine.indexOf("<") + 1, inputLine.indexOf(">"));
            String[] status = status_output.split("\\|");

            GlobalData.status = status[0];
            //System.out.println("Status: " + GlobalData.status);

            String mpos = status[1].substring(status[1].indexOf("MPos:") + 5, status[1].length());
            //System.out.println("pos data: " + mpos);
            String[] dro = mpos.split(",");
            GlobalData.dro[0] = Float.parseFloat(dro[0]);
            GlobalData.dro[1] = Float.parseFloat(dro[1]);
            GlobalData.dro[2] = Float.parseFloat(dro[2]);
            //System.out.println("X: " + GlobalData.dro[0] + " Y: " + GlobalData.dro[1] + " Z: " + GlobalData.dro[2]);
            if (GlobalData.last_dro[0] == GlobalData.dro[0] && GlobalData.last_dro[1] == GlobalData.dro[1] && GlobalData.last_dro[2] == GlobalData.dro[2])
            {
                GlobalData.IsInMotion = false;
                if (GlobalData.pendingReset == true)
                {
                    MotionController.SoftResetNow();
                }
            }
            else
            {
                GlobalData.IsInMotion = true;
            }
            if (GlobalData.last_dro[0] == GlobalData.dro[0])
            {
                GlobalData.IsXAxisInMotion = false;
            }
            else
            {
                GlobalData.IsXAxisInMotion = true;
            }
            if (GlobalData.last_dro[1] == GlobalData.dro[1])
            {
                GlobalData.IsYAxisInMotion = false;
            }
            else
            {
                GlobalData.IsYAxisInMotion = true;
            }
            if (GlobalData.last_dro[2] == GlobalData.dro[2])
            {
                GlobalData.IsZAxisInMotion = false;
            }
            else
            {
                GlobalData.IsZAxisInMotion = true;
            }
            //System.out.println("IsInMotion: " + GlobalData.IsInMotion);

        }
        else if (inputLine.contains("[") && inputLine.contains("]")) //Work Offset Parameters
        {
            String parameter_output = inputLine.substring(inputLine.indexOf("[") + 1, inputLine.indexOf("]"));
            String[] g_param = parameter_output.split(":");
            //System.out.println("Param: " + g_param[0]);
            if (g_param[0].equals("G92"))
            {
                String[] pos = g_param[1].split(",");
                GlobalData.work_offset[0] = Float.parseFloat(pos[0]);
                GlobalData.work_offset[1] = Float.parseFloat(pos[1]);
                GlobalData.work_offset[2] = Float.parseFloat(pos[2]);
            }
        }
    }
    public static void WriteWait() {
        try {
            Thread.sleep(100);
        } catch (InterruptedException ex) {
            Thread.currentThread().interrupt();
        }
    }
    public void SetJogSpeed(float jog)
    {
        jog_speed = jog;
    }
    public static void CycleStart()
    {
        WriteBuffer("~\n");
    }
    public static void FeedHold()
    {
        WriteBuffer("!\n");
        //serial.write("!\n");
    }
    public static void SoftResetNow()
    {
        WriteWait();
        serial.write("\030");
        serial.write("\n");
        WriteWait();
        WriteWait();
        serial.write("$X");
    }
    public static void SoftReset()
    {
        GlobalData.pendingReset = true;
    }
    public static void JogX_Plus()
    {
        WriteBuffer("$J=G91 F" + jog_speed + " G20 X20\n");
    }
    public static void JogX_Minus()
    {
        WriteBuffer("$J=G91 F" + jog_speed + " G20 X-20\n");
    }

    public static void JogY_Plus()
    {
        WriteBuffer("$J=G91 F" + jog_speed + " G20 Y20\n");
    }
    public static void JogY_Minus()
    {
        WriteBuffer("$J=G91 F" + jog_speed + " G20 Y-20\n");
    }

    public static void JogZ_Plus()
    {
        WriteBuffer("$J=G91 F" + jog_speed + " G20 Z20\n");
    }
    public static void JogZ_Minus()
    {
        WriteBuffer("$J=G91 F" + jog_speed + " G20 Z-20\n");
    }

    public static void EndJog()
    {
        FeedHold();
    }

    /* Begin Default Parameters */
    public static int step_len = 10; //uSec
    public static int step_idle_delay =  25; //mSec
    public static int step_port_invert_mask =  0; //00000000
    public static int dir_port_invert_mask =  6; //00000110
    public static boolean step_enable_invert = true;
    public static boolean limit_pins_invert = true;
    public static boolean probe_pins_invert = true;
    public static int status_report_mask = 3; //00000011
    public static float junction_deviation = 0.020f; //millimeter
    public static float arc_tolorance = 0.002f; //millimeter
    public static boolean report_inches = true;
    public static boolean soft_limits = false;
    public static boolean hard_limits = false;
    public static boolean homing_cycle = false;
    public static int homing_dir_invert = 1; //00000001
    public static float homing_feed = 50f; //mm/min
    public static float homing_seek = 635f; //mm/min
    public static int homing_debounce = 250; //mSec
    public static float homing_pull_off = 1.00f; //millimeter
    public static float x_step_scale = 300f; //step/mm
    public static float y_step_scale = 300f; //step/mm
    public static float z_step_scale = 300f; //step/mm
    public static float x_max_velocity = 15240f; //mm/min
    public static float y_max_velocity = 15240f; //mm/min
    public static float z_max_velocity = 15240f; //mm/min
    public static float x_accel = 381; //mm/sec^2
    public static float y_accel = 381f; //mm/sec^2
    public static float z_accel = 127f; //mm/sec^2
    public static float x_max_travel = 0f; //millimeter
    public static float y_max_travel = 0f; //millimeter
    public static float z_max_travel = 0f; //millimeter

    public static void InitMotionController()
    {
        WriteBuffer("$13=" + report_inches + "\n");

        x_step_scale = GlobalData.X_Scale * 25.4f;
        WriteBuffer("$100=" + x_step_scale + "\n");
        y_step_scale = GlobalData.Y_Scale * 25.4f;
        WriteBuffer("$101=" + y_step_scale + "\n");
        z_step_scale = GlobalData.Z_Scale * 25.4f;
        WriteBuffer("$102=" + z_step_scale + "\n");

        x_max_velocity = GlobalData.X_Max_Vel * 25.4f;
        WriteBuffer("$110=" + x_max_velocity + "\n");
        y_max_velocity = GlobalData.Y_Max_Vel * 25.4f;
        WriteBuffer("$111=" + y_max_velocity + "\n");
        z_max_velocity = GlobalData.Z_Max_Vel * 25.4f;
        WriteBuffer("$112=" + z_max_velocity + "\n");

        x_accel = GlobalData.X_Accel * 25.4f;
        WriteBuffer("$110=" + x_accel + "\n");
        y_accel = GlobalData.Y_Accel * 25.4f;
        WriteBuffer("$111=" + y_accel + "\n");
        z_accel = GlobalData.Z_Accel * 25.4f;
        WriteBuffer("$112=" + z_accel + "\n");

        x_max_travel = GlobalData.X_Extents * 25.4f;
        WriteBuffer("$130=" + x_max_travel + "\n");
        y_max_travel = GlobalData.Y_Extents * 25.4f;
        WriteBuffer("$131=" + y_max_travel + "\n");
        z_max_travel = GlobalData.Z_Extents * 25.4f;
        WriteBuffer("$132=" + z_max_travel + "\n");
    }
}
