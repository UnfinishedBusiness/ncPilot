/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.nc.pilot.lib.MotionController;

import com.google.gson.Gson;
import com.nc.pilot.lib.GlobalData;
import com.nc.pilot.lib.MDIConsole.MDIConsole;
import com.nc.pilot.lib.SerialIO;
import com.nc.pilot.lib.UIWidgets.UIWidgets;

import java.io.IOException;
import java.util.ArrayList;

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
    private static float lastExecutionLine = 0;
    private static UIWidgets ui_widgets;
    private static MDIConsole mdi_console;
    private static boolean WaitingForStopMotion = false;
    private static Runnable RunAfterStop = null;
    public static int BlockNextStatusReports = 0;
    public MotionController(SerialIO s)
    {
        serial = s;
    }
    private static float jog_speed = 0;
    public static void inherit_ui_widgets(UIWidgets u)
    {
        ui_widgets = u;
    }
    public static void inherit_mdi_console(MDIConsole m)
    {
        mdi_console = m;
    }

    private static float lastGword;
    private static float lastXword;
    private static float lastYword;
    private static float lastZword;
    private static float lastFword;
    private static float lastIword;
    private static float lastJword;

    private static float Gword;
    private static float Xword;
    private static float Yword;
    private static float Zword;
    private static float Fword;
    private static float Iword;
    private static float Jword;

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
    public static ArrayList<float[]> getPointsOfArc(float[] start, float[] end, float[] center, float radius, String direction)
    {
        float start_angle = getAngle(center, start);
        float end_angle = getAngle(center, end);
        float angle_inc = 1;
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
    public static void WriteBuffer(String data){
        //GlobalData.WriteBuffer.add(data);
        serial.write(data);
    }
    private static void ParseQueReport(Integer q)
    {
        //System.out.println("Buffer Available: " + q);
        //GlobalData.FreeBuffers = q;

        if (q > 10) //If qr is > 30
        {
            //GlobalData.PlannerReady = true;
        }
        else
        {
            //GlobalData.PlannerReady = false;
        }
    }
    public static void ReadBuffer(String inputLine){
        //System.out.println("Read line: " + inputLine);
        mdi_console.RecieveBufferLine(inputLine);
        Gson g = new Gson();
        Gson qr = new Gson();
        Report report = qr.fromJson(inputLine, Report.class);
        if (report != null)
        {
            if (report.qr != null)
            {
                //ParseQueReport(Integer.parseInt(report.qr));
            }
        }
        JSON_Data json = g.fromJson(inputLine, JSON_Data.class);
        if (json != null)
        {
            if (json.r != null)
            {
                GlobalData.LinesToSend = 1;
                json.sr = json.r.sr;
            }
            if (json.sr != null)
            {
                if (BlockNextStatusReports == 0)
                {
                    if (json.sr.posx != null)
                    {
                        GlobalData.dro[0] = Float.parseFloat(json.sr.posx);
                    }
                    if (json.sr.posy != null)
                    {
                        GlobalData.dro[1] = Float.parseFloat(json.sr.posy);
                    }
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
                if (BlockNextStatusReports == 0)
                {
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
                if (json.sr.line != null)
                {
                    lastExecutionLine = GlobalData.CurrentExecutionLine;
                    GlobalData.CurrentExecutionLine = Integer.parseInt(json.sr.line);
                    //System.out.println("Current Execution Line: " + GetGcodeLineAtN(GlobalData.CurrentExecutionLine));
                    for (float x = lastExecutionLine; x < GlobalData.CurrentExecutionLine; x++)
                    {
                        String Line = GetGcodeLineAtN((int)x);
                        float Mword = getGword(Line, 'M');
                        if (Mword != -1f)
                        {
                            //System.out.println("Found M word: " + Mword);
                            if (Mword == 3)
                            {
                                ui_widgets.engageButton("Torch On", true);
                                ui_widgets.engageButton("Torch Off", false);
                            }
                            if (Mword == 5)
                            {
                                ui_widgets.engageButton("Torch On", false);
                                ui_widgets.engageButton("Torch Off", true);
                            }
                        }
                    }
                }
                if (json.sr.stat != null)
                {
                    int stat = Integer.parseInt(json.sr.stat);
                    //System.out.println("New Status: " + stat);
                    if (stat == 0) GlobalData.MachineState = "Init";
                    if (stat == 1) GlobalData.MachineState = "Ready";
                    if (stat == 2) GlobalData.MachineState = "Alarm";
                    if (stat == 3)
                    {
                        GlobalData.MachineState = "Stop";
                        if (RunAfterStop != null)
                        {
                            Runnable once = RunAfterStop;
                            RunAfterStop = null;
                            once.run();
                        }
                    }
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
    public static void WriteWait(int ms) {
        try {
            Thread.sleep(ms);
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
        GlobalData.LinesToSend = 4;
        LoadGcodeFile();
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
        GlobalData.GcodeFileCurrentLine = 0;
        GlobalData.LinesToSend = 0;
        GlobalData.GcodeFileLines = null;
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
        if (BlockNextStatusReports == 0)
        {
            WriteBuffer("{\"sr\":\"\"}\n");
        }
        else
        {
            BlockNextStatusReports--;
            if (BlockNextStatusReports < 0) BlockNextStatusReports = 0;
        }
    }
    public static void Home()
    {
        WriteBuffer("G28.3 X=0 Y=0 Z=0\n");
        //WriteWait();
        //WriteWait();
        //WriteWait();
        ///WriteWait();
        //WriteWait();
        StatusReport();
    }
    public static float getGword(String line, char Word)
    {
        boolean capture = false;
        String word_builder = "";
        for (int x = 0; x < line.length(); x++)
        {
            if (line.charAt(x) == '(')
            {
                //Found comment
                break;
            }
            if (capture == true)
            {
                if (Character.isDigit(line.charAt(x)) || line.charAt(x) == '.' || line.charAt(x) == '-')
                {
                    word_builder = word_builder + line.charAt(x);
                }
                if ((Character.isAlphabetic(line.charAt(x)) && line.charAt(x) != ' ') || x == line.length() - 1)
                {
                    if (word_builder != "")
                    {
                        float word = new Float(word_builder);
                        return word;
                    }
                    capture = false;
                    word_builder = "";
                }
            }
            if (line.charAt(x) == Word)
            {
                capture = true;
            }
        }
        return -1f;
    }
    public static String GetGcodeLineAtN(int n)
    {
        for (int x = 0; x < GlobalData.GcodeFileLines.length; x++)
        {
            if (getGword(GlobalData.GcodeFileLines[x], 'N') == n)
            {
                return GlobalData.GcodeFileLines[x];
            }
        }
        return "";
    }
    public static void WriteBufferAndRunAfterStop(String mdi, Runnable run)
    {
        WriteBuffer(mdi);
        RunAfterStop = run;
    }
    public static void updateGcodeRegisters(String line, char Word)
    {
        boolean capture = false;
        String word_builder = "";
        for (int x = 0; x < line.length(); x++)
        {
            if (line.charAt(x) == '(')
            {
                //Found comment
                break;
            }
            if (capture == true)
            {
                if (Character.isDigit(line.charAt(x)) || line.charAt(x) == '.' || line.charAt(x) == '-')
                {
                    word_builder = word_builder + line.charAt(x);
                }
                if ((Character.isAlphabetic(line.charAt(x)) && line.charAt(x) != ' ') || x == line.length() - 1)
                {
                    if (word_builder != "")
                    {
                        float word = new Float(word_builder);
                        if (Word == 'g')
                        {
                            Gword = word;
                        }
                        if (Word == 'x')
                        {
                            Xword = word;
                        }
                        if (Word == 'y')
                        {
                            Yword = word;
                        }
                        if (Word == 'z')
                        {
                            Zword = word;
                        }
                        if (Word == 'i')
                        {
                            Iword = word;
                        }
                        if (Word == 'j')
                        {
                            Jword = word;
                        }
                        if (Word == 'f')
                        {
                            Fword = word;
                        }
                    }
                    capture = false;
                    word_builder = "";
                }
            }
            if (line.charAt(x) == Word)
            {
                capture = true;
            }
        }
    }
    public static void updateLastGcodeRegisters(String line, char Word)
    {
        boolean capture = false;
        String word_builder = "";
        for (int x = 0; x < line.length(); x++)
        {
            if (line.charAt(x) == '(')
            {
                //Found comment
                break;
            }
            if (capture == true)
            {
                if (Character.isDigit(line.charAt(x)) || line.charAt(x) == '.' || line.charAt(x) == '-')
                {
                    word_builder = word_builder + line.charAt(x);
                }
                if ((Character.isAlphabetic(line.charAt(x)) && line.charAt(x) != ' ') || x == line.length() - 1)
                {
                    if (word_builder != "")
                    {
                        float word = new Float(word_builder);
                        if (Word == 'g')
                        {
                            lastGword = word;
                        }
                        if (Word == 'x')
                        {
                            lastXword = word;
                        }
                        if (Word == 'y')
                        {
                            lastYword = word;
                        }
                        if (Word == 'z')
                        {
                            lastZword = word;
                        }
                        if (Word == 'i')
                        {
                            lastIword = word;
                        }
                        if (Word == 'j')
                        {
                            lastJword = word;
                        }
                        if (Word == 'f')
                        {
                            lastFword = word;
                        }
                    }
                    capture = false;
                    word_builder = "";
                }
            }
            if (line.charAt(x) == Word)
            {
                capture = true;
            }
        }
    }
    public static void LoadGcodeFile()
    {
        try {
            String buffer = GlobalData.readFile(GlobalData.GcodeFile);
            String[] lines = buffer.split("\n");

            ArrayList<String> gcode = new ArrayList();
            for (int x = 0; x < lines.length; x++)
            {
                updateGcodeRegisters(lines[x].toLowerCase(), 'g');
                updateGcodeRegisters(lines[x].toLowerCase(), 'x');
                updateGcodeRegisters(lines[x].toLowerCase(), 'y');
                updateGcodeRegisters(lines[x].toLowerCase(), 'z');
                updateGcodeRegisters(lines[x].toLowerCase(), 'i');
                updateGcodeRegisters(lines[x].toLowerCase(), 'j');
                updateGcodeRegisters(lines[x].toLowerCase(), 'f');
                if (lines[x].toLowerCase().contains("m30"))
                {
                    gcode.add("M5");
                    gcode.add("M9");
                    gcode.add("G80");
                    gcode.add("G90");
                    gcode.add("G94");
                }
                else if (lines[x].toLowerCase().contains("o<touchoff>"))
                {
                    String touchoff = lines[x].toLowerCase().substring(lines[x].toLowerCase().indexOf("o<touchoff> ") + 12);
                    //System.out.println("Touchoff String: " + touchoff);
                    String[] touchoff_split = touchoff.split("\\s+");
                    if (touchoff_split.length > 2)
                    {
                        String pierce_height = touchoff_split[1].substring(1, (touchoff_split[1].length() - 1));
                        String pierce_delay = touchoff_split[2].substring(1, (touchoff_split[2].length() - 1));
                        String cut_height = touchoff_split[3].substring(1, (touchoff_split[3].length() - 1));
                        //System.out.println("TouchOff-> Pierce Height: " + pierce_height + " Pierce Delay: " + pierce_delay + " Cut Height: " + cut_height);
                        gcode.add("M9 G28.2 Z0");
                        gcode.add("G92 Z=0");
                        gcode.add("G1 Z" + pierce_height + " F50");
                        gcode.add("M3S2000 G4 P" + pierce_delay);
                        gcode.add("G1 Z" + cut_height);
                        gcode.add("M8");
                    }
                }
                else if (Gword == 2) //Clockwise arc - Convert to line segments
                {
                    float[] center = new float[]{lastXword + Iword, lastYword + Jword};
                    float radius = new Float(Math.hypot(Xword-center[0], Yword-center[1]));
                    ArrayList<float[]> arc_points = getPointsOfArc(new float[]{lastXword, lastYword}, new float[]{Xword, Yword}, center, radius, "CW");
                    for (int y = 0; y < arc_points.size(); y+= 20)
                    {
                        gcode.add("G1 X" + arc_points.get(y)[0] + " Y" + arc_points.get(y)[1]);
                    }
                }
                else if (Gword == 3 && lastXword != Xword && lastYword != Yword) //Counter-Clockwise arc - Convert to line segments
                {
                    float[] center = new float[]{lastXword + Iword, lastYword + Jword};
                    float radius = new Float(Math.hypot(Xword-center[0], Yword-center[1]));
                    ArrayList<float[]> arc_points = getPointsOfArc(new float[]{lastXword, lastYword}, new float[]{Xword, Yword}, center, radius, "CCW");
                    for (int y = 0; y < arc_points.size(); y+= 20)
                    {
                        gcode.add("G1 X" + arc_points.get(y)[0] + " Y" + arc_points.get(y)[1]);
                    }
                }
                else
                {
                    gcode.add(lines[x]);
                }
                updateLastGcodeRegisters(lines[x].toLowerCase(), 'g');
                updateLastGcodeRegisters(lines[x].toLowerCase(), 'x');
                updateLastGcodeRegisters(lines[x].toLowerCase(), 'y');
                updateLastGcodeRegisters(lines[x].toLowerCase(), 'z');
                updateLastGcodeRegisters(lines[x].toLowerCase(), 'i');
                updateLastGcodeRegisters(lines[x].toLowerCase(), 'j');
                updateLastGcodeRegisters(lines[x].toLowerCase(), 'f');
            }
            GlobalData.GcodeFileLines = new String[gcode.size()];
            for (int x = 0; x < gcode.size(); x++)
            {
                GlobalData.GcodeFileLines[x] = gcode.get(x);
                System.out.println(gcode.get(x));
            }


        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    public static void Poll()
    {
        while (GlobalData.LinesToSend > 0) //We have lines to send to the controller
        {
            if (GlobalData.GcodeFileLines != null)
            {
                if (GlobalData.GcodeFileCurrentLine < GlobalData.GcodeFileLines.length)
                {
                    System.out.println("Writing line: " + GlobalData.GcodeFileLines[GlobalData.GcodeFileCurrentLine]);
                    WriteBuffer(GlobalData.GcodeFileLines[GlobalData.GcodeFileCurrentLine] + "\n");
                    GlobalData.GcodeFileCurrentLine++;
                }
            }
            GlobalData.LinesToSend--;
        }
    }
    /*public static void Poll_Free_Buffer_Method()
    {
        if (GlobalData.RunCycle == true && WaitingForStopMotion == false)
        {
            ui_widgets.engageButton("Start", true);
        }
        if (GlobalData.MachineState.contentEquals("Stop") && WaitingForStopMotion == true)
        {
            ui_widgets.engageButton("Start", false);
            Abort();
            WaitingForStopMotion = false;
        }
        if (GlobalData.RunCycle == true && GlobalData.GcodeFileSendLine < GlobalData.GcodeFileLines.length)
        {
            if (GlobalData.FreeBuffers > 5)
            {
                if (GlobalData.GcodeFileLines[GlobalData.GcodeFileSendLine].toLowerCase().contains("m30"))
                {
                    GlobalData.GcodeFileLines[GlobalData.GcodeFileSendLine] = "M5\nM9\nG80\nG90\nG94\n";
                    WaitingForStopMotion = true;
                }
                else if (GlobalData.GcodeFileLines[GlobalData.GcodeFileSendLine].toLowerCase().contains("o<touchoff>"))
                {
                    String touchoff = GlobalData.GcodeFileLines[GlobalData.GcodeFileSendLine].toLowerCase().substring(GlobalData.GcodeFileLines[GlobalData.GcodeFileSendLine].toLowerCase().indexOf("o<touchoff> ") + 12);
                    //System.out.println("Touchoff String: " + touchoff);
                    String[] touchoff_split = touchoff.split("\\s+");
                    if (touchoff_split.length > 2)
                    {
                        String pierce_height = touchoff_split[1].substring(1, (touchoff_split[1].length() - 1));
                        String pierce_delay = touchoff_split[2].substring(1, (touchoff_split[2].length() - 1));
                        String cut_height = touchoff_split[3].substring(1, (touchoff_split[3].length() - 1));
                        //System.out.println("TouchOff-> Pierce Height: " + pierce_height + " Pierce Delay: " + pierce_delay + " Cut Height: " + cut_height);
                        WriteBuffer("M9\n");
                        WriteBuffer("G38.2 Z-10 F50\n");
                        WriteBuffer("G92 Z=-0.375\n");
                        WriteBuffer("G90 G0 Z" + pierce_height + "\n");
                        WriteBuffer("M3S2000\n");
                        WriteBuffer("G4 P" + pierce_delay + "\n");
                        WriteBuffer("G1 Z" + cut_height + "\n");
                        WriteBuffer("M8\n");
                        GlobalData.GcodeFileSendLine++;
                    }

                }
                else
                {
                    WriteBuffer(GlobalData.GcodeFileLines[GlobalData.GcodeFileSendLine]);
                    GlobalData.GcodeFileSendLine++;
                }
            }

        }

    }*/
    public static void InitMotionController()
    {
        WriteBuffer("$ej=1\n");
        WriteBuffer("$ej=1\n");
        WriteBuffer("{\"sr\":{\"line\":true, \"posx\":true, \"posy\":true, \"posz\":true, \"mpox\":true, \"mpoy\":true, \"mpoz\":true, \"ofsx\":true, \"ofsy\":true, \"ofsz\":true, \"feed\": true, \"vel\":true, \"unit\":true, \"stat\":true}}\n");
        //WriteBuffer("G54\n");
    }

}
