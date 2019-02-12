/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.nc.pilot.lib.MotionController;

import com.google.gson.Gson;
import com.nc.pilot.lib.GlobalData;
import com.nc.pilot.lib.SerialIO;
import com.nc.pilot.lib.ncConfig;
import com.nc.pilot.ui.MachineControl;

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
    private static float jog_speed = 0;

    public static void WriteBuffer(String data){
        //GlobalData.WriteBuffer.add(data);
        serial.write(data);
    }
    private static void ParseQueReport(Integer q)
    {
        System.out.println("Buffer Available: " + q);
        GlobalData.FreeBuffers = q;
        if (q > 10) //If qr is > 30
        {
            GlobalData.PlannerReady = true;
        }
        else
        {
            GlobalData.PlannerReady = false;
        }
    }
    public static void ReadBuffer(String inputLine){
        System.out.println("Read line: " + inputLine);
        Gson g = new Gson();
        Gson qr = new Gson();
        Report report = qr.fromJson(inputLine, Report.class);
        if (report != null)
        {
            if (report.qr != null)
            {
                ParseQueReport(Integer.parseInt(report.qr));
            }
        }

        JSON_Data json = g.fromJson(inputLine, JSON_Data.class);
        //System.out.println(json.posy);
        if (json != null)
        {
            if (json.sr != null)
            {
                if (json.sr.posx != null)
                {
                    GlobalData.dro[0] = Float.parseFloat(json.sr.posx);
                }
                if (json.sr.posy != null)
                {
                    GlobalData.dro[1] = Float.parseFloat(json.sr.posy);
                }
                if (json.sr.posz != null)
                {
                    GlobalData.dro[2] = Float.parseFloat(json.sr.posz);
                }
                if (json.sr.vel != null)
                {
                    GlobalData.CurrentVelocity = Float.parseFloat(json.sr.vel);
                }
            }
            if (json.r != null)
            {
                if (json.r.qr != null)
                {
                    ParseQueReport(Integer.parseInt(json.r.qr));
                }
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
    public static void Abort()
    {
        WriteBuffer("%\n");
    }
    public static void JogX_Plus()
    {
        WriteBuffer("G91 G20 G1 X" + GlobalData.X_Extents * 2 + " F" + jog_speed + "\n");
    }
    public static void JogX_Minus()
    {
        WriteBuffer("G91 G20 G1 X-" + GlobalData.X_Extents * 2 + " F" + jog_speed + "\n");
    }

    public static void JogY_Plus()
    {
        WriteBuffer("G91 G20 G1 Y" + GlobalData.Y_Extents * 2 + " F" + jog_speed + "\n");
    }
    public static void JogY_Minus()
    {
        WriteBuffer("G91 G20 G1 Y-" + GlobalData.Y_Extents * 2 + " F" + jog_speed + "\n");
    }

    public static void JogZ_Plus()
    {
        WriteBuffer("G91 G20 G1 Z" + GlobalData.Z_Extents * 2 + " F" + jog_speed + "\n");
    }
    public static void JogZ_Minus()
    {
        WriteBuffer("G91 G20 G1 Z-" + GlobalData.Z_Extents * 2 + " F" + jog_speed + "\n");
    }

    public static void EndJog()
    {
        FeedHold();
        Abort();
    }

}
