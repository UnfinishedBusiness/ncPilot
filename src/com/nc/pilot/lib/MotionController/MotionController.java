/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.nc.pilot.lib.MotionController;

import com.nc.pilot.lib.GlobalData;
import com.nc.pilot.lib.MDIConsole.MDIConsole;
import com.nc.pilot.lib.UIWidgets.UIWidgets;

import java.io.IOException;
import java.util.ArrayList;

import com.fazecast.jSerialComm.*;

/**
 *
 * @author travis
 */

public class MotionController {

    /* End Default Parameters */
    private SerialPort comPort;
    private String rx_buffer_line;
    private UIWidgets ui_widgets;
    private MDIConsole mdi_console;
    private float jog_speed = 0;
    public boolean JogX = false;
    public boolean JogXdir = false;
    public boolean JogY = false;
    public boolean JogYdir = false;
    public boolean JogZ = false;
    public boolean JogZdir = false;

    String[] ErrorValues = new String[]{
            "", //there is no error 0
            "G-code words consist of a letter and a value. Letter was not found.",
            "Numeric value format is not valid or missing an expected value.",
            "Xmotion system command was not recognized or supported.",
            "Negative value received for an expected positive value.",
            "Homing cycle is not enabled via settings.",
            "Minimum step pulse time must be greater than 3usec",
            "EEPROM read failed. Reset and restored to default values.",
            "Command cannot be used unless Xmotion is IDLE. Ensures smooth operation during a job.",
            "G-code locked out during alarm or jog state",
            "Soft limits cannot be enabled without homing also enabled.",
            "Max characters per line exceeded. Line was not processed and executed.",
            "Setting value exceeds the maximum step rate supported.",
            "Safety door detected as opened and door state initiated.",
            "Build info or startup line exceeded EEPROM line length limit.",
            "Jog target exceeds machine travel. Command ignored.",
            "Jog command with no '=' or contains prohibited g-code.",
            "Laser mode requires PWM output.",
            "Unsupported or invalid g-code command found in block.",
            "More than one g-code command from same modal group found in block.",
            "Feed rate has not yet been set or is undefined.",
            "G-code command in block requires an integer value.",
            "Two G-code commands that both require the use of the XYZ axis words were detected in the block.",
            "A G-code word was repeated in the block.",
            "A G-code command implicitly or explicitly requires XYZ axis words in the block, but none were detected.",
            "N line number value is not within the valid range of 1 - 9,999,999.",
            "A G-code command was sent, but is missing some required P or L value words in the line.",
            "Xmotion supports six work coordinate systems G54-G59. G59.1, G59.2, and G59.3 are not supported.",
            "The G53 G-code command requires either a G0 seek or G1 feed motion mode to be active. A different motion was active.",
            "There are unused axis words in the block and G80 motion mode cancel is active.",
            "A G2 or G3 arc was commanded but there are no XYZ axis words in the selected plane to trace the arc.",
            "The motion command has an invalid target. G2, G3, and G38.2 generates this error, if the arc is impossible to generate or if the probe target is the current position.",
            "A G2 or G3 arc, traced with the radius definition, had a mathematical error when computing the arc geometry. Try either breaking up the arc into semi-circles or quadrants, or redefine them with the arc offset definition.",
            "A G2 or G3 arc, traced with the offset definition, is missing the IJK offset word in the selected plane to trace the arc.",
            " There are unused, leftover G-code words that aren't used by any command in the block.",
            "The G43.1 dynamic tool length offset command cannot apply an offset to an axis other than its configured axis. The Grbl default axis is the Z-axis.",
            "Tool number greater than max supported value.",
    };

    public void inherit_ui_widgets(UIWidgets u)
    {
        ui_widgets = u;
    }
    public void inherit_mdi_console(MDIConsole m)
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

    public MotionController() {
        SerialPort[] ports = SerialPort.getCommPorts();
        for (int x = 0; x < ports.length; x++)
        {
            System.out.println(x + "> Port Name: " + ports[x].getSystemPortName() + " Port Description: " + ports[x].getDescriptivePortName());
            if (ports[x].getSystemPortName().contentEquals("ttyACM0"))
            {
                comPort = ports[x];
                comPort.setBaudRate(115200);
                comPort.openPort();
                rx_buffer_line = "";

                ResetNow();
                WriteBuffer("?\n");
            }
        }
    }

    private boolean inTolerance(float a, float b, float t)
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
    public float getAngle(float[] start_point, float[] end_point) {
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
    public float getLineLength(float[] start_point, float[] end_point)
    {
        return new Float(Math.hypot(start_point[0]-end_point[0], start_point[1]-end_point[1]));
    }
    public float[] rotatePoint(float[] pivot, float[] rotated_point, float angle)
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
    public float[] getPolarLineEndpoint(float[] start_point, float length, float angle)
    {
        float[] end_point = new float[] {start_point[0] + length, start_point[1]};
        return rotatePoint(start_point, end_point, angle);
    }
    public ArrayList<float[]> getPointsOfArc(float[] start, float[] end, float[] center, float radius, String direction)
    {
        float start_angle = getAngle(center, start);
        float end_angle = getAngle(center, end);
        float angle_inc = 1;
        ArrayList<float[]> points = new ArrayList();
        points.add(start);
        //System.out.println("start_angle: " + start_angle + " end_angle: " + end_angle);
        if (start_angle == end_angle) //We are a circle
        {
            if (direction == "CCW")
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
                for (float x = 360; x > 0; x -= angle_inc)
                {
                    start_angle -= angle_inc;
                    float [] new_point = getPolarLineEndpoint(center, radius, start_angle);
                    points.add(new_point);
                }
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
    public void StatusReport()
    {
        WriteBuffer("?\n");
    }
    public void WriteBuffer(String data){
        comPort.writeBytes(data.getBytes(), data.length());
    }
    public void SetJogSpeed(float jog)
    {
        jog_speed = jog;
    }
    public void CycleStart()
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
    public void FeedHold()
    {
        WriteBuffer("!\n");
    }
    public void Abort()
    {
        ResetOnIdle();
        GlobalData.GcodeFileCurrentLine = 0;
        GlobalData.GcodeFileLines = null;
    }
    public void ResetOnIdle()
    {
        GlobalData.ResetOnIdle = true;
        FeedHold();
    }
    public void ResetNow()
    {
        System.out.println("ResetNow!");
        GlobalData.ResetOnIdle = false;
        comPort.writeBytes(new byte[]{ 0x18 }, 1);
        WriteBuffer("~\n");

    }
    public void JogX_Plus()
    {
        if (GlobalData.JogMode.contentEquals("0.1"))  WriteBuffer("G91 G20 G1 X" + 0.1 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.01"))  WriteBuffer("G91 G20 G1 X" + 0.01 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.001"))  WriteBuffer("G91 G20 G1 X" + 0.001 + " F" + jog_speed + "\n");
    }
    public void JogX_Minus()
    {
        if (GlobalData.JogMode.contentEquals("0.1")) WriteBuffer("G91 G20 G1 X-" + 0.1 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.01")) WriteBuffer("G91 G20 G1 X-" + 0.01 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.001")) WriteBuffer("G91 G20 G1 X-" + 0.001 + " F" + jog_speed + "\n");
    }

    public void JogY_Plus()
    {
        if (GlobalData.JogMode.contentEquals("0.1")) WriteBuffer("G91 G20 G1 Y" + 0.1 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.01")) WriteBuffer("G91 G20 G1 Y" + 0.01 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.001")) WriteBuffer("G91 G20 G1 Y" + 0.001 + " F" + jog_speed + "\n");
    }
    public void JogY_Minus()
    {
        if (GlobalData.JogMode.contentEquals("0.1")) WriteBuffer("G91 G20 G1 Y-" + 0.1 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.01")) WriteBuffer("G91 G20 G1 Y-" + 0.01 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.001")) WriteBuffer("G91 G20 G1 Y-" + 0.001 + " F" + jog_speed + "\n");
    }

    public void JogZ_Plus()
    {
        if (GlobalData.JogMode.contentEquals("0.1")) WriteBuffer("G91 G20 G1 Z" + 0.1 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.01")) WriteBuffer("G91 G20 G1 Z" + 0.01 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.001")) WriteBuffer("G91 G20 G1 Z" + 0.001 + " F" + jog_speed + "\n");
    }
    public void JogZ_Minus()
    {
        if (GlobalData.JogMode.contentEquals("0.1")) WriteBuffer("G91 G20 G1 Z-" + 0.1 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.01")) WriteBuffer("G91 G20 G1 Z-" + 0.01 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.001")) WriteBuffer("G91 G20 G1 Z-" + 0.001 + " F" + jog_speed + "\n");
    }

    public void EndJog()
    {
        int cancel = 0x85;
        byte[] data = {0, 0};
        data[0] = (byte) (cancel & 0xFF);
        data[1] = (byte) ((cancel >> 8) & 0xFF);
        comPort.writeBytes(data, 2);
    }
    public void SetXzero()
    {
        WriteBuffer("G10 L20 P1 X0\n");
    }
    public void SetYzero()
    {
        WriteBuffer("G10 L20 P1 Y0\n");
    }
    public void SetZzero()
    {
        WriteBuffer("G10 L20 P1 Z0\n");
    }
    public void Home()
    {

    }
    public float getGword(String line, char Word)
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
    public String GetGcodeLineAtN(int n)
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
    public void updateGcodeRegisters(String line, char Word)
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
    public void updateLastGcodeRegisters(String line, char Word)
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
    public void LoadGcodeFile()
    {
        try {
            String buffer = GlobalData.readFile(GlobalData.GcodeFile);
            String[] lines = buffer.split("\n");

            ArrayList<String> gcode = new ArrayList();
            for (int x = 0; x < lines.length; x++)
            {
                if (lines[x].toLowerCase().contains("m"))
                {
                    gcode.add(lines[x]);
                }
                else
                {
                    updateGcodeRegisters(lines[x].toLowerCase(), 'g');
                    updateGcodeRegisters(lines[x].toLowerCase(), 'x');
                    updateGcodeRegisters(lines[x].toLowerCase(), 'y');
                    updateGcodeRegisters(lines[x].toLowerCase(), 'z');
                    updateGcodeRegisters(lines[x].toLowerCase(), 'i');
                    updateGcodeRegisters(lines[x].toLowerCase(), 'j');
                    updateGcodeRegisters(lines[x].toLowerCase(), 'f');
                    if (lines[x].toLowerCase().contains("g64"))
                    {
                        //Just filter out G64 to avoid error
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
                            gcode.add("F30");
                            gcode.add("M9"); //Turn of ATHC
                            //gcode.add("G38.3 Z-10"); //Probe Until Touch
                            //gcode.add("G91 G0 Z0.1875"); //Takeup slack in floating head
                            gcode.add("G90"); //Switch to absolute
                            //gcode.add("G10 L20 P1 Z0"); //Set Z0 as top of sheet
                            gcode.add("G1 Z" + pierce_height); //Raise to pierce height
                            gcode.add("M3 S5000"); //Turn on plasma
                            gcode.add("G4 P" + pierce_delay); //Pierce Delay
                            gcode.add("G1 Z" + cut_height); //Traverse to Cut Height
                            gcode.add("G90"); //Switch to absolute
                            gcode.add("M8"); //Turn on ATHC
                        }
                    }
                    /*else if (Gword == 2) //Clockwise arc - Convert to line segments
                    {
                        if (lastXword != Xword || lastYword != Yword || lastIword != Iword || lastJword != Jword)
                        {
                            float[] center = new float[]{lastXword + Iword, lastYword + Jword};
                            float radius = new Float(Math.hypot(Xword-center[0], Yword-center[1]));
                            ArrayList<float[]> arc_points = getPointsOfArc(new float[]{lastXword, lastYword}, new float[]{Xword, Yword}, center, radius, "CW");
                            for (int y = 0; y < arc_points.size(); y+= 30)
                            {
                                gcode.add("G1 X" + arc_points.get(y)[0] + " Y" + arc_points.get(y)[1] + " F" + Fword);
                            }
                            gcode.add("G1 X" + Xword + " Y" + Yword + " F" + Fword);
                        }
                    }
                    else if (Gword == 3) //Counter-Clockwise arc - Convert to line segments
                    {
                        if (lastXword != Xword || lastYword != Yword || lastIword != Iword || lastJword != Jword)
                        {
                            float[] center = new float[]{lastXword + Iword, lastYword + Jword};
                            float radius = new Float(Math.hypot(Xword-center[0], Yword-center[1]));
                            ArrayList<float[]> arc_points = getPointsOfArc(new float[]{lastXword, lastYword}, new float[]{Xword, Yword}, center, radius, "CCW");
                            for (int y = 0; y < arc_points.size(); y+= 30)
                            {
                                gcode.add("G1 X" + arc_points.get(y)[0] + " Y" + arc_points.get(y)[1] + " F" + Fword);
                            }
                            gcode.add("G1 X" + Xword + " Y" + Yword + " F" + Fword);
                        }
                    }*/
                /*else if (Gword == 0 || Gword == 1 || Gword == 2 || Gword == 3)
                {

                }*/
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
    public void ReadBuffer(String inputLine){
        mdi_console.RecieveBufferLine(inputLine);
        //System.out.println(inputLine);
        if (inputLine.contains("ok"))
        {
            //System.out.println("Setting SendLine Flag!");
            GlobalData.SendLines = 1;
        }
        else if (inputLine.contains("error"))
        {
            //Figure out what error it is and notify. Serious errors need to hold machine
            //System.out.println("Setting SendLine Flag!");
            ReportError(inputLine);
            //System.out.println("Found Error, halted!");
            //FeedHold();
            GlobalData.SendLines = 1;
        }
        else if (inputLine.contains("PRB")) //Probing cycle finished
        {
            //System.out.println("Probing cycle touched! Continuing stream!");
            //ResetOnIdle();
            GlobalData.ProbingCycleActive = false;
            GlobalData.SendLines = 1;
        }
        else if (inputLine.contains("Hold Complete") && GlobalData.ResetOnIdle == true)
        {
            ResetNow();
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
                        String[] wo_pos = pairs[x].substring(4).split(",");
                        GlobalData.work_offset[0] = new Float(wo_pos[0]);
                        GlobalData.work_offset[1] = new Float(wo_pos[1]);
                        GlobalData.work_offset[2] = new Float(wo_pos[2]);

                        GlobalData.dro[0] = GlobalData.machine_cordinates[0] - GlobalData.work_offset[0];
                        GlobalData.dro[1] = GlobalData.machine_cordinates[1] - GlobalData.work_offset[1];
                        GlobalData.dro[2] = GlobalData.machine_cordinates[2] - GlobalData.work_offset[2];
                    }
                    else if (pairs[x].contains("FS"))
                    {
                        GlobalData.CurrentVelocity = new Float(pairs[x].substring(3).split(",")[0]);
                    }
                }
            }
        }
    }
    public void Poll()
    {
        if (comPort.bytesAvailable() > 0)
        {
            byte[] readBuffer = new byte[comPort.bytesAvailable()];
            int numRead = comPort.readBytes(readBuffer, readBuffer.length);
            //System.out.println("Read " + numRead + " bytes.");
            for (int x = 0; x < numRead; x++)
            {
                char c = new Character((char)readBuffer[x]).charValue();
                if (c != '\r') //Ignore carrage returns
                {
                    if (c == '\n')
                    {
                        //System.out.println("Found line break!");
                        if (rx_buffer_line.length() > 0)
                        {
                            ReadBuffer(rx_buffer_line);
                        }
                        rx_buffer_line = "";
                    }
                    else
                    {
                        //System.out.println("Concatting: " + c);
                        rx_buffer_line = rx_buffer_line + c;
                        //System.out.println("rx_buffer_line: " + rx_buffer_line);
                    }
                }
            }

        }
        if (GlobalData.JogMode.contentEquals("Continuous") && (GlobalData.SendLines > 0 || GlobalData.MachineState.contentEquals("Idle")))
        {
            if (JogX == true || JogY == true || JogZ == true)
            {
                GlobalData.MachineState = "Jog";
                String jog_string = "$J=G20 G91 F + " + jog_speed + " ";
                float jog_inc_dist = 1f * (jog_speed / 6000);
                if (jog_inc_dist > 0.150f) jog_inc_dist = 0.150f;
                if (jog_inc_dist < 0.001f) jog_inc_dist = 0.001f;
                if (JogX == true)
                {
                    //System.out.println("Jog X!");
                    if (JogXdir == true) //Jog Positive
                    {
                        jog_string += "X" + jog_inc_dist +  " ";
                    }
                    else //Jog negative
                    {
                        jog_string += "X-" + jog_inc_dist + " ";
                    }
                }

                if (JogY == true)
                {
                    //System.out.println("Jog Y!");
                    if (JogYdir == true) //Jog Positive
                    {
                        jog_string += "Y" + jog_inc_dist +  " ";
                    }
                    else //Jog negative
                    {
                        jog_string += "Y-" + jog_inc_dist +  " ";
                    }
                }

                if (JogZ == true)
                {
                    //System.out.println("Jog Z!");
                    if (JogZdir == true) //Jog Positive
                    {
                        jog_string += "Z" + jog_inc_dist +  " ";
                    }
                    else //Jog negative
                    {
                        jog_string += "Z-" + jog_inc_dist +  " ";
                    }
                }
                //System.out.println("Writing jog string: " + jog_string);
                WriteBuffer(jog_string + "\n");
                GlobalData.SendLines--;
            }
            else
            {
                if (GlobalData.MachineState.contentEquals("Jog"))
                {
                    GlobalData.MachineState = "Idle";
                    EndJog();
                }
            }
        }
        while (GlobalData.SendLines > 0)
        {
            if (GlobalData.GcodeFileLines != null)
            {
                //System.out.println("{Poll} Sending Line!");
                if (GlobalData.GcodeFileCurrentLine < GlobalData.GcodeFileLines.length) {

                    //System.out.println("Writing line: " + GlobalData.GcodeFileLines[GlobalData.GcodeFileCurrentLine]);
                    if (GlobalData.ProbingCycleActive == false) //Stop writing gcode to planner until after probing cycle is finished
                    {
                        WriteBuffer(GlobalData.GcodeFileLines[GlobalData.GcodeFileCurrentLine] + "\n");
                        if (GlobalData.GcodeFileLines[GlobalData.GcodeFileCurrentLine].toLowerCase().contains("m30"))
                        {
                            System.out.println("Found end of program, resetting program!");
                            GlobalData.GcodeFileLines = null;
                            GlobalData.GcodeFileCurrentLine = 0;
                            break;
                        }
                    }
                    if (GlobalData.GcodeFileLines[GlobalData.GcodeFileCurrentLine].toLowerCase().contains("g38"))
                    {
                        //System.out.println("Found probing cycle, waiting for touch before sending more lines!");
                        GlobalData.ProbingCycleActive = true;
                    }
                    GlobalData.GcodeFileCurrentLine++;
                }
            }
            GlobalData.SendLines--;
        }



    }

    private void ReportError(String line)
    {
        int error = new Integer(line.substring(6));
        if (ErrorValues.length > error)
        {
            System.out.println("Error: " + ErrorValues[error]);
        }
        else
        {
            System.out.println("Unknown Error!");
        }
    }
}
