/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.nc.pilot.lib.MotionController;

import com.google.gson.Gson;
import com.nc.pilot.lib.GcodeInterpreter;
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
    public static void ReadBuffer(String inputLine){
        mdi_console.RecieveBufferLine(inputLine);
        System.out.println(inputLine);
        if (inputLine.contains("ok"))
        {
            System.out.println("Setting SendLine Flag!");
            GlobalData.SendLines++;
        }
        else if (inputLine.contains("error"))
        {
            //Figure out what error it is and notify. Serious errors need to hold machine
            System.out.println("Setting SendLine Flag!");
            GlobalData.SendLines++;
        }
        String report = inputLine.substring(1, inputLine.length()-1);
        if (report == "") return;
        if (inputLine.charAt(0) == '<') //We are a report
        {
            String[] pairs = report.split("\\|");
            if (pairs.length > 0)
            {
                GlobalData.MachineState = pairs[0];
                for (int x = 1; x < pairs.length; x++)
                {
                    if (pairs[x].contains("MPos"))
                    {
                        String[] abs_pos = pairs[x].substring(5).split(",");
                        GlobalData.machine_cordinates[0] = new Float(abs_pos[0]);
                        GlobalData.machine_cordinates[1] = new Float(abs_pos[1]);
                        GlobalData.machine_cordinates[2] = new Float(abs_pos[2]);

                        GlobalData.dro[0] = GlobalData.machine_cordinates[0] - GlobalData.work_offset[0];
                        GlobalData.dro[1] = GlobalData.machine_cordinates[1] - GlobalData.work_offset[1];
                        GlobalData.dro[2] = GlobalData.machine_cordinates[2] - GlobalData.work_offset[2];
                    }
                    else if (pairs[x].contains("WCO"))
                    {
                        String[] wo_pos = pairs[x].substring(5).split(",");
                        GlobalData.work_offset[0] = new Float(wo_pos[0]);
                        GlobalData.work_offset[1] = new Float(wo_pos[1]);
                        GlobalData.work_offset[2] = new Float(wo_pos[2]);
                    }
                    else if (pairs[x].contains("FS"))
                    {
                        GlobalData.CurrentVelocity = new Float(pairs[x].substring(3).split(",")[0]);
                    }
                }
            }
        }
    }
    public void SetJogSpeed(float jog)
    {
        jog_speed = jog;
    }
    public static void CycleStart()
    {
        WriteBuffer("~");
        if (GlobalData.GcodeFileLines == null)
        {
            LoadGcodeFile();
            if (GlobalData.GcodeFileCurrentLine < GlobalData.GcodeFileLines.length) {
                System.out.println("Writing line: " + GlobalData.GcodeFileLines[GlobalData.GcodeFileCurrentLine]);
                WriteBuffer(GlobalData.GcodeFileLines[GlobalData.GcodeFileCurrentLine] + "\n");
                GlobalData.GcodeFileCurrentLine++;
            }
        }
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
        GlobalData.GcodeFileLines = null;
    }
    public static void JogX_Plus()
    {
        //if (GlobalData.JogMode.contentEquals("Continuous"))  WriteBuffer("G91 G20 G1 X" + GlobalData.X_Extents * 2 + " F" + jog_speed + "\n");
        //if (GlobalData.JogMode.contentEquals("0.1"))  WriteBuffer("G91 G20 G1 X" + 0.1 + " F" + jog_speed + "\n");
        //if (GlobalData.JogMode.contentEquals("0.01"))  WriteBuffer("G91 G20 G1 X" + 0.01 + " F" + jog_speed + "\n");
        //if (GlobalData.JogMode.contentEquals("0.001"))  WriteBuffer("G91 G20 G1 X" + 0.001 + " F" + jog_speed + "\n");
    }
    public static void JogX_Minus()
    {
        //if (GlobalData.JogMode.contentEquals("Continuous")) WriteBuffer("G91 G20 G1 X-" + GlobalData.X_Extents * 2 + " F" + jog_speed + "\n");
        //if (GlobalData.JogMode.contentEquals("0.1")) WriteBuffer("G91 G20 G1 X-" + 0.1 + " F" + jog_speed + "\n");
        //if (GlobalData.JogMode.contentEquals("0.01")) WriteBuffer("G91 G20 G1 X-" + 0.01 + " F" + jog_speed + "\n");
        //if (GlobalData.JogMode.contentEquals("0.001")) WriteBuffer("G91 G20 G1 X-" + 0.001 + " F" + jog_speed + "\n");
    }

    public static void JogY_Plus()
    {
        //if (GlobalData.JogMode.contentEquals("Continuous")) WriteBuffer("G91 G20 G1 Y" + GlobalData.Y_Extents * 2 + " F" + jog_speed + "\n");
        //if (GlobalData.JogMode.contentEquals("0.1")) WriteBuffer("G91 G20 G1 Y" + 0.1 + " F" + jog_speed + "\n");
        //if (GlobalData.JogMode.contentEquals("0.01")) WriteBuffer("G91 G20 G1 Y" + 0.01 + " F" + jog_speed + "\n");
        //if (GlobalData.JogMode.contentEquals("0.001")) WriteBuffer("G91 G20 G1 Y" + 0.001 + " F" + jog_speed + "\n");
    }
    public static void JogY_Minus()
    {
        //if (GlobalData.JogMode.contentEquals("Continuous")) WriteBuffer("G91 G20 G1 Y-" + GlobalData.Y_Extents * 2 + " F" + jog_speed + "\n");
        //if (GlobalData.JogMode.contentEquals("0.1")) WriteBuffer("G91 G20 G1 Y-" + 0.1 + " F" + jog_speed + "\n");
        //if (GlobalData.JogMode.contentEquals("0.01")) WriteBuffer("G91 G20 G1 Y-" + 0.01 + " F" + jog_speed + "\n");
        //if (GlobalData.JogMode.contentEquals("0.001")) WriteBuffer("G91 G20 G1 Y-" + 0.001 + " F" + jog_speed + "\n");
    }

    public static void JogZ_Plus()
    {
        //if (GlobalData.JogMode.contentEquals("Continuous")) WriteBuffer("G91 G20 G1 Z" + GlobalData.Z_Extents * 2 + " F" + jog_speed + "\n");
        //if (GlobalData.JogMode.contentEquals("0.1")) WriteBuffer("G91 G20 G1 Z" + 0.1 + " F" + jog_speed + "\n");
        //if (GlobalData.JogMode.contentEquals("0.01")) WriteBuffer("G91 G20 G1 Z" + 0.01 + " F" + jog_speed + "\n");
        //if (GlobalData.JogMode.contentEquals("0.001")) WriteBuffer("G91 G20 G1 Z" + 0.001 + " F" + jog_speed + "\n");
    }
    public static void JogZ_Minus()
    {
        //if (GlobalData.JogMode.contentEquals("Continuous")) WriteBuffer("G91 G20 G1 Z-" + GlobalData.Z_Extents * 2 + " F" + jog_speed + "\n");
        //if (GlobalData.JogMode.contentEquals("0.1")) WriteBuffer("G91 G20 G1 Z-" + 0.1 + " F" + jog_speed + "\n");
        //if (GlobalData.JogMode.contentEquals("0.01")) WriteBuffer("G91 G20 G1 Z-" + 0.01 + " F" + jog_speed + "\n");
        //if (GlobalData.JogMode.contentEquals("0.001")) WriteBuffer("G91 G20 G1 Z-" + 0.001 + " F" + jog_speed + "\n");
    }

    public static void EndJog()
    {
        FeedHold();
        Abort();
    }
    public static void SetXzero()
    {
        //WriteBuffer("G92 X=0\n");
        //StatusReport();
    }
    public static void SetYzero()
    {
        //WriteBuffer("G92 Y=0\n");
        //StatusReport();
    }
    public static void SetZzero()
    {
        //WriteBuffer("G92 Z=0\n");
        //StatusReport();
    }
    public static void Home()
    {

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
        String buffer = null;
        try {
            buffer = GlobalData.readFile(GlobalData.GcodeFile);
            String[] lines = buffer.split("\n");
            ArrayList<String> gcode = new ArrayList();
            for (int x = 0; x < lines.length; x++)
            {
                if (lines[x].toLowerCase().contains("o<touchoff>"))
                {
                    String touchoff = lines[x].toLowerCase().substring(lines[x].toLowerCase().indexOf("o<touchoff> ") + 12);
                    //System.out.println("Touchoff String: " + touchoff);
                    String[] touchoff_split = touchoff.split("\\s+");
                    if (touchoff_split.length > 2)
                    {
                        String pierce_height = touchoff_split[1].substring(1, (touchoff_split[1].length() - 1));
                        String pierce_delay = touchoff_split[2].substring(1, (touchoff_split[2].length() - 1));
                        String cut_height = touchoff_split[3].substring(1, (touchoff_split[3].length() - 1));
                        System.out.println("TouchOff-> Pierce Height: " + pierce_height + " Pierce Delay: " + pierce_delay + " Cut Height: " + cut_height);
                        //gcode.add("F10");
                        //gcode.add("M9"); //Turn of ATHC
                        //gcode.add("G38.3 Z-10"); //Probe Until Touch
                        //gcode.add("G91"); //Switch to incremental
                        //gcode.add("G0 Z0.1875"); //Takeup slack in floating head
                        //gcode.add("G90"); //Switch to absolute
                        //gcode.add("G10 L20 P1 Z0"); //Set Z0 as top of sheet
                        //gcode.add("G1 Z" + pierce_height); //Raise to pierce height
                        //gcode.add("M3S5000"); //Turn on plasma
                        //gcode.add("G4 P" + pierce_delay); //Pierce Delay
                        //gcode.add("G1 Z" + cut_height); //Traverse to Cut Height
                        //gcode.add("M8"); //Turn on ATHC
                    }
                }
                else if (lines[x].toLowerCase().contains("G64")) //We don't support G64 any more
                {

                }
                else
                {
                    gcode.add(lines[x]);
                }

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
    public static void LoadGcodeFile_NoInlineMods()
    {
        String buffer = null;
        try {
            buffer = GlobalData.readFile(GlobalData.GcodeFile);
            GlobalData.GcodeFileLines = buffer.split("\n");
        } catch (IOException e) {
            e.printStackTrace();
        }

    }
    public static void LoadGcodeFile_Bad()
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
                        //gcode.add("M9 G28.2 Z0");
                        //gcode.add("G92 Z=0");
                        //gcode.add("G1 Z" + pierce_height + " F50");
                        //gcode.add("M3S2000 G4 P" + pierce_delay);
                        //gcode.add("G1 Z" + cut_height);
                        //gcode.add("M8");
                    }
                }
                /*else if (Gword == 2) //Clockwise arc - Convert to line segments
                {
                    if (lastXword != Xword || lastYword != Yword || lastIword != Iword || lastJword != Jword)
                    {
                        float[] center = new float[]{lastXword + Iword, lastYword + Jword};
                        float radius = new Float(Math.hypot(Xword-center[0], Yword-center[1]));
                        ArrayList<float[]> arc_points = getPointsOfArc(new float[]{lastXword, lastYword}, new float[]{Xword, Yword}, center, radius, "CW");
                        for (int y = 0; y < arc_points.size(); y+= 5)
                        {
                            gcode.add("G1 X" + arc_points.get(y)[0] + " Y" + arc_points.get(y)[1]);
                        }
                        gcode.add("G1 X" + Xword + " Y" + Yword);
                    }
                }*/
                /*else if (Gword == 3) //Counter-Clockwise arc - Convert to line segments
                {
                    if (lastXword != Xword || lastYword != Yword || lastIword != Iword || lastJword != Jword)
                    {
                        float[] center = new float[]{lastXword + Iword, lastYword + Jword};
                        float radius = new Float(Math.hypot(Xword-center[0], Yword-center[1]));
                        ArrayList<float[]> arc_points = getPointsOfArc(new float[]{lastXword, lastYword}, new float[]{Xword, Yword}, center, radius, "CCW");
                        for (int y = 0; y < arc_points.size(); y+= 5)
                        {
                            gcode.add("G1 X" + arc_points.get(y)[0] + " Y" + arc_points.get(y)[1]);
                        }
                    }
                }*/
                else if (Gword == 0 || Gword == 1 || Gword == 2 || Gword == 3)
                {

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
        while (GlobalData.SendLines > 0)
        {
            if (GlobalData.GcodeFileLines != null)
            {
                System.out.println("{Poll} Sending Line! " + GlobalData.SendLines);
                if (GlobalData.GcodeFileCurrentLine < GlobalData.GcodeFileLines.length) {
                    System.out.println("Writing line: " + GlobalData.GcodeFileLines[GlobalData.GcodeFileCurrentLine]);
                    WriteBuffer(GlobalData.GcodeFileLines[GlobalData.GcodeFileCurrentLine] + "\n");
                    GlobalData.GcodeFileCurrentLine++;
                }
            }
            GlobalData.SendLines--;
        }
    }
}
