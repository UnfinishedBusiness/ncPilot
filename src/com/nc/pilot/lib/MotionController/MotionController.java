/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.nc.pilot.lib.MotionController;

import com.google.gson.Gson;
import com.nc.pilot.lib.GlobalData;
import com.nc.pilot.lib.SerialIO;

import java.io.IOException;

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
        //System.out.println("Buffer Available: " + q);
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

        if (json != null)
        {
            if (json.r != null)
            {
                json.sr = json.r.sr;
            }
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
                if (json.sr.mpox != null)
                {
                    if (GlobalData.CurrentUnits.contentEquals("Inch"))
                    {
                        GlobalData.machine_cordinates[0] = Float.parseFloat(json.sr.mpox) / 25.4f;
                    }
                    else
                    {
                        GlobalData.machine_cordinates[0] = Float.parseFloat(json.sr.mpox);
                    }
                }
                if (json.sr.mpoy != null)
                {
                    if (GlobalData.CurrentUnits.contentEquals("Inch"))
                    {
                        GlobalData.machine_cordinates[1] = Float.parseFloat(json.sr.mpoy) / 25.4f;
                    }
                    else
                    {
                        GlobalData.machine_cordinates[1] = Float.parseFloat(json.sr.mpoy);
                    }
                }
                if (json.sr.mpoz != null)
                {
                    if (GlobalData.CurrentUnits.contentEquals("Inch"))
                    {
                        GlobalData.machine_cordinates[2] = Float.parseFloat(json.sr.mpoz) / 25.4f;
                    }
                    else
                    {
                        GlobalData.machine_cordinates[2] = Float.parseFloat(json.sr.mpoz);
                    }
                }
                if (json.sr.ofsx != null)
                {
                    if (GlobalData.CurrentUnits.contentEquals("Inch"))
                    {
                        GlobalData.work_offset[0] = Float.parseFloat(json.sr.ofsx) / 25.4f;
                    }
                    else
                    {
                        GlobalData.work_offset[0] = Float.parseFloat(json.sr.ofsx);
                    }
                    //System.out.println("Set X work offset to: " + GlobalData.work_offset[0]);
                }
                if (json.sr.ofsy != null)
                {
                    if (GlobalData.CurrentUnits.contentEquals("Inch"))
                    {
                        GlobalData.work_offset[1] = Float.parseFloat(json.sr.ofsy) / 25.4f;
                    }
                    else
                    {
                        GlobalData.work_offset[1] = Float.parseFloat(json.sr.ofsy);
                    }
                    //System.out.println("Set Y work offset to: " + GlobalData.work_offset[1]);
                }
                if (json.sr.ofsz != null)
                {
                    if (GlobalData.CurrentUnits.contentEquals("Inch"))
                    {
                        GlobalData.work_offset[2] = Float.parseFloat(json.sr.ofsz) / 25.4f;
                    }
                    else
                    {
                        GlobalData.work_offset[2] = Float.parseFloat(json.sr.ofsz);
                    }
                    //System.out.println("Set Z work offset to: " + GlobalData.work_offset[2]);
                }
                if (json.sr.vel != null)
                {
                    GlobalData.CurrentVelocity = Float.parseFloat(json.sr.vel);
                }
                if (json.sr.feed != null)
                {
                    GlobalData.ProgrammedFeedrate = Float.parseFloat(json.sr.feed);
                }
                if (json.sr.stat != null)
                {
                    int stat = Integer.parseInt(json.sr.stat);
                    //System.out.println("New Status: " + stat);
                    if (stat == 0) GlobalData.MachineState = "Init";
                    if (stat == 1) GlobalData.MachineState = "Ready";
                    if (stat == 2) GlobalData.MachineState = "Alarm";
                    if (stat == 3) GlobalData.MachineState = "Stop";
                    if (stat == 4) GlobalData.MachineState = "End";
                    if (stat == 5) GlobalData.MachineState = "Motion";
                    if (stat == 6) GlobalData.MachineState = "Hold";
                    if (stat == 7) GlobalData.MachineState = "Probe";
                    if (stat == 8) GlobalData.MachineState = "Cycle";
                    if (stat == 8) GlobalData.MachineState = "Homing";
                }
                if (json.sr.unit != null)
                {
                    if (Integer.parseInt(json.sr.unit) == 0) GlobalData.CurrentUnits = "Inch";
                    if (Integer.parseInt(json.sr.unit) == 1) GlobalData.CurrentUnits = "Metric";
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
        GlobalData.RunCycle = true;
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
        GlobalData.GcodeFileExecutionLine = 0;
    }
    public static void JogX_Plus()
    {
        if (GlobalData.JogMode.contentEquals("Continuous"))  WriteBuffer("G91 G20 G1 X" + GlobalData.X_Extents * 2 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.1"))  WriteBuffer("G91 G20 G1 X" + 0.1 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.01"))  WriteBuffer("G91 G20 G1 X" + 0.01 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.001"))  WriteBuffer("G91 G20 G1 X" + 0.001 + " F" + jog_speed + "\n");
    }
    public static void JogX_Minus()
    {
        if (GlobalData.JogMode.contentEquals("Continuous")) WriteBuffer("G91 G20 G1 X-" + GlobalData.X_Extents * 2 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.1")) WriteBuffer("G91 G20 G1 X-" + 0.1 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.01")) WriteBuffer("G91 G20 G1 X-" + 0.01 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.001")) WriteBuffer("G91 G20 G1 X-" + 0.001 + " F" + jog_speed + "\n");
    }

    public static void JogY_Plus()
    {
        if (GlobalData.JogMode.contentEquals("Continuous")) WriteBuffer("G91 G20 G1 Y" + GlobalData.Y_Extents * 2 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.1")) WriteBuffer("G91 G20 G1 Y" + 0.1 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.01")) WriteBuffer("G91 G20 G1 Y" + 0.01 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.001")) WriteBuffer("G91 G20 G1 Y" + 0.001 + " F" + jog_speed + "\n");
    }
    public static void JogY_Minus()
    {
        if (GlobalData.JogMode.contentEquals("Continuous")) WriteBuffer("G91 G20 G1 Y-" + GlobalData.Y_Extents * 2 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.1")) WriteBuffer("G91 G20 G1 Y-" + 0.1 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.01")) WriteBuffer("G91 G20 G1 Y-" + 0.01 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.001")) WriteBuffer("G91 G20 G1 Y-" + 0.001 + " F" + jog_speed + "\n");
    }

    public static void JogZ_Plus()
    {
        if (GlobalData.JogMode.contentEquals("Continuous")) WriteBuffer("G91 G20 G1 Z" + GlobalData.Z_Extents * 2 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.1")) WriteBuffer("G91 G20 G1 Z" + 0.1 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.01")) WriteBuffer("G91 G20 G1 Z" + 0.01 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.001")) WriteBuffer("G91 G20 G1 Z" + 0.001 + " F" + jog_speed + "\n");
    }
    public static void JogZ_Minus()
    {
        if (GlobalData.JogMode.contentEquals("Continuous")) WriteBuffer("G91 G20 G1 Z-" + GlobalData.Z_Extents * 2 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.1")) WriteBuffer("G91 G20 G1 Z-" + 0.1 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.01")) WriteBuffer("G91 G20 G1 Z-" + 0.01 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.001")) WriteBuffer("G91 G20 G1 Z-" + 0.001 + " F" + jog_speed + "\n");
    }

    public static void EndJog()
    {
        FeedHold();
        Abort();
    }
    public static void SetXzero()
    {
        WriteBuffer("G92 X=0\n");
        StatusReport();
    }
    public static void SetYzero()
    {
        WriteBuffer("G92 Y=0\n");
        StatusReport();
    }
    public static void SetZzero()
    {
        WriteBuffer("G92 Z=0\n");
        StatusReport();
    }
    public static void StatusReport()
    {
        WriteBuffer("{\"sr\":\"\"}\n");
    }
    public static void Home()
    {
        WriteBuffer("G28.3 X=0 Y=0 Z=0\"");
        //WriteWait();
        //WriteWait();
        //WriteWait();
        ///WriteWait();
        //WriteWait();
        StatusReport();
    }
    public static void LoadGcodeFile(String filename)
    {
        try {
            String buffer = GlobalData.readFile(filename);
            String[] lines = buffer.split("\n");
            GlobalData.GcodeFileLines = lines;
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    public static void Poll()
    {
        if (GlobalData.RunCycle == true && GlobalData.GcodeFileExecutionLine < GlobalData.GcodeFileLines.length)
        {
            if (GlobalData.FreeBuffers > 10)
            {
                WriteBuffer(GlobalData.GcodeFileLines[GlobalData.GcodeFileExecutionLine]);
                GlobalData.GcodeFileExecutionLine++;
            }
        }
    }
    public static void InitMotionController()
    {
        WriteBuffer("$ej=1\n");
        WriteBuffer("$ej=1\n");
        WriteBuffer("{\"sr\":{\"line\":true, \"posx\":true, \"posy\":true, \"posz\":true, \"mpox\":true, \"mpoy\":true, \"mpoz\":true, \"ofsx\":true, \"ofsy\":true, \"ofsz\":true, \"feed\": true, \"vel\":true, \"unit\":true, \"stat\":true}}\n");
        //WriteBuffer("G54\n");
    }

}
