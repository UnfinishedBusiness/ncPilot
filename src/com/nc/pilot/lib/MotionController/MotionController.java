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
    private boolean OkayToSend = false;


    public void inherit_ui_widgets(UIWidgets u)
    {
        ui_widgets = u;
    }
    public void inherit_mdi_console(MDIConsole m)
    {
        mdi_console = m;
    }

    private static int checksum(byte[] buf, int len)
    {
        int checksum = 0;
        int count = len;
        while (count > 0)
        {
            checksum ^= buf[--count];
        }
        return checksum;
    }

    public MotionController() {
        SerialPort[] ports = SerialPort.getCommPorts();
        for (int x = 0; x < ports.length; x++) {
            System.out.println(x + "> Port Name: " + ports[x].getSystemPortName() + " Port Description: " + ports[x].getDescriptivePortName());
            //if (ports[x].getSystemPortName().contentEquals("COM11") && ports[x].getDescriptivePortName().contentEquals("USBSER001"))
            if (ports[x].getSystemPortName().contentEquals("COM11")) {
                comPort = ports[x];
                comPort.setBaudRate(250000);
                comPort.openPort();
                rx_buffer_line = "";

                WriteBuffer("G20\n");
            }
        }
    }
    public void WriteBuffer(String data){
        //System.out.println("WriteBuffer: " + data);
        comPort.writeBytes(data.getBytes(), data.length());
    }
    public void SetJogSpeed(float jog)
    {
        jog_speed = jog;
    }
    public void CycleStart()
    {
        if (GlobalData.GcodeFileLines == null)
        {
            WriteBuffer("M110 N0\n");
            WriteBuffer("M28 0.nc\n");
            GlobalData.GcodeFileCurrentLine = 0;
            LoadGcodeFile();
            OkayToSend = true; //Send first line with number
            ui_widgets.setVisability("Gcode Upload", true);
        }
        else
        {
            WriteBuffer("M24\n");
        }
    }
    public void FeedHold()
    {
        WriteBuffer("M25\n");
    }
    public void Abort()
    {
        GlobalData.GcodeFileCurrentLine = 0;
        GlobalData.GcodeFileLines = null;
        WriteBuffer("M25\nM26 S0\n");
        //WriteBuffer("M2101");
    }
    public void JogX_Plus()
    {
        if (GlobalData.JogMode.contentEquals("0.1"))  WriteBuffer("G91 G20 G1 X" + 0.1 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.01"))  WriteBuffer("G91 G20 G1 X" + 0.01 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.001"))  WriteBuffer("G91 G20 G1 X" + 0.001 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("Continuous"))  WriteBuffer("M3000 P0 S" + jog_speed + " D1\n");
    }
    public void JogX_Minus()
    {
        if (GlobalData.JogMode.contentEquals("0.1")) WriteBuffer("G91 G20 G1 X-" + 0.1 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.01")) WriteBuffer("G91 G20 G1 X-" + 0.01 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.001")) WriteBuffer("G91 G20 G1 X-" + 0.001 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("Continuous"))  WriteBuffer("M3000 P0 S" + jog_speed + " D-1\n");
    }

    public void JogY_Plus()
    {
        if (GlobalData.JogMode.contentEquals("0.1")) WriteBuffer("G91 G20 G1 Y" + 0.1 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.01")) WriteBuffer("G91 G20 G1 Y" + 0.01 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.001")) WriteBuffer("G91 G20 G1 Y" + 0.001 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("Continuous"))  WriteBuffer("M3000 P1 S" + jog_speed + " D1\n");
    }
    public void JogY_Minus()
    {
        if (GlobalData.JogMode.contentEquals("0.1")) WriteBuffer("G91 G20 G1 Y-" + 0.1 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.01")) WriteBuffer("G91 G20 G1 Y-" + 0.01 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.001")) WriteBuffer("G91 G20 G1 Y-" + 0.001 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("Continuous"))  WriteBuffer("M3000 P1 S" + jog_speed + " D-1\n");
    }

    public void JogZ_Plus()
    {
        if (GlobalData.JogMode.contentEquals("0.1")) WriteBuffer("G91 G20 G1 Z" + 0.1 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.01")) WriteBuffer("G91 G20 G1 Z" + 0.01 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.001")) WriteBuffer("G91 G20 G1 Z" + 0.001 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("Continuous"))  WriteBuffer("M3000 P2 S" + jog_speed + " D1\n");
    }
    public void JogZ_Minus()
    {
        if (GlobalData.JogMode.contentEquals("0.1")) WriteBuffer("G91 G20 G1 Z-" + 0.1 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.01")) WriteBuffer("G91 G20 G1 Z-" + 0.01 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("0.001")) WriteBuffer("G91 G20 G1 Z-" + 0.001 + " F" + jog_speed + "\n");
        if (GlobalData.JogMode.contentEquals("Continuous"))  WriteBuffer("M3000 P2 S" + jog_speed + " D-1\n");
    }

    public void EndXJog()
    {
        if (GlobalData.JogMode.contentEquals("Continuous"))  WriteBuffer("M3001 P0\n");
    }
    public void EndYJog()
    {
        if (GlobalData.JogMode.contentEquals("Continuous"))  WriteBuffer("M3001 P1\n");
    }
    public void EndZJog()
    {
        if (GlobalData.JogMode.contentEquals("Continuous"))  WriteBuffer("M3001 P2\n");
    }
    public void SetXzero()
    {
        WriteBuffer("G92 X0\n");
    }
    public void SetYzero()
    {
        WriteBuffer("G92 Y0\n");
    }
    public void SetZzero()
    {
        WriteBuffer("G92 Z0\n");
    }
    public void Home()
    {

    }


    public void LoadGcodeFile()
    {
        try {
            String buffer = GlobalData.readFile(GlobalData.GcodeFile);
            String[] lines = buffer.split("\n");
            GlobalData.GcodeFileLines = lines;
            for (int x = 0; x < GlobalData.GcodeFileLines.length; x++)
            {
                String line = "N" + (x +1) + " " + GlobalData.GcodeFileLines[x];
                char[] new_line = line.toCharArray();
                String new_string = "";
                for (int y = 0; y < new_line.length; y++)
                {
                    if (Character.isAlphabetic(new_line[y]) || Character.isDigit(new_line[y]) || Character.isSpaceChar(new_line[y]) || new_line[y] == '-' || new_line[y] == '.')
                    {
                        new_string = new_string + new_line[y];
                    }
                }
                String sum = String.valueOf(checksum(new_string.getBytes(), new_string.length()));
                //System.out.println(new_string + "*" + sum);
                GlobalData.GcodeFileLines[x] = new_string + "*" + sum;
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    public void ReadBuffer(String inputLine){
        //mdi_console.RecieveBufferLine(inputLine); //Only do this if we wan't to log all input
        if (inputLine.contains("Checksum") && inputLine.contains("ok"))
        {
            //mdi_console.RecieveBufferLine("(MotionController) Recieved Checksum OK! Okay to send next line!");
            OkayToSend = true;
        }
        else if (inputLine.contains("DRO"))
        {
            String dro_line = inputLine.split("DRO:\\ ")[1];
            String[] axis_pairs = dro_line.split("\\ ");
            for (int x = 0; x < axis_pairs.length; x++)
            {
                //System.out.println(axis_pairs[x]);
                if (axis_pairs[x].contains("X_WO"))
                {
                    GlobalData.work_offset[0] = new Float(axis_pairs[x].split("\\=")[1]);
                }
                if (axis_pairs[x].contains("Y_WO"))
                {
                    GlobalData.work_offset[1] = new Float(axis_pairs[x].split("\\=")[1]);
                }
                if (axis_pairs[x].contains("Z_WO"))
                {
                    GlobalData.work_offset[2] = new Float(axis_pairs[x].split("\\=")[1]);
                }
                if (axis_pairs[x].contains("X_MCS"))
                {
                    GlobalData.machine_cordinates[0] = new Float(axis_pairs[x].split("\\=")[1]);
                    GlobalData.dro[0] = GlobalData.machine_cordinates[0] + GlobalData.work_offset[0];
                }
                if (axis_pairs[x].contains("Y_MCS"))
                {
                    GlobalData.machine_cordinates[1] = new Float(axis_pairs[x].split("\\=")[1]);
                    GlobalData.dro[1] = GlobalData.machine_cordinates[1] + GlobalData.work_offset[1];
                }
                if (axis_pairs[x].contains("Z_MCS"))
                {
                    GlobalData.machine_cordinates[2] = new Float(axis_pairs[x].split("\\=")[1]);
                    GlobalData.dro[2] = GlobalData.machine_cordinates[2] + GlobalData.work_offset[2];
                }
                if (axis_pairs[x].contains("UNITS"))
                {
                    GlobalData.CurrentUnits = axis_pairs[x].split("\\=")[1];
                }
                if (axis_pairs[x].contains("FEEDRATE"))
                {
                    GlobalData.ProgrammedFeedrate = new Float(axis_pairs[x].split("\\=")[1]);
                }
                if (axis_pairs[x].contains("VELOCITY"))
                {
                    GlobalData.CurrentVelocity = new Float(axis_pairs[x].split("\\=")[1]);
                }
                if (axis_pairs[x].contains("STATUS"))
                {
                    GlobalData.MachineState = axis_pairs[x].split("\\=")[1];
                }
                if (axis_pairs[x].contains("THC_ARC_VOLTAGE"))
                {
                    GlobalData.CurrentArcVoltage = new Float(axis_pairs[x].split("\\=")[1]);
                }
                if (axis_pairs[x].contains("THC_SET_VOLTAGE"))
                {
                    GlobalData.SetArcVoltage = new Float(axis_pairs[x].split("\\=")[1]);
                }
            }
        }
        else
        {
            mdi_console.RecieveBufferLine(inputLine);
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
        if (GlobalData.GcodeFileLines != null) //If Gcode file is open
        {
            //System.out.println("Gcode file is open and we are at line: " + GlobalData.GcodeFileCurrentLine);
            if (OkayToSend == true)
            {
                OkayToSend = false; //Only send one line per "Checksum ok"
                if  (GlobalData.GcodeFileCurrentLine < GlobalData.GcodeFileLines.length)
                {
                    WriteBuffer(GlobalData.GcodeFileLines[GlobalData.GcodeFileCurrentLine] + "\n");
                    float upload_percentage = ((float)GlobalData.GcodeFileCurrentLine / (float)GlobalData.GcodeFileLines.length) * 100.0f;
                    //mdi_console.RecieveBufferLine("Wrote: " + GlobalData.GcodeFileLines[GlobalData.GcodeFileCurrentLine] + " (" + upload_percentage + "%)\n");
                    ui_widgets.setSliderPosition("Gcode Upload", upload_percentage);
                    //System.out.println("Wrote: " + GlobalData.GcodeFileLines[GlobalData.GcodeFileCurrentLine] + "\n");
                    GlobalData.GcodeFileCurrentLine++;
                }
                else //End of file reached. Null it and send the finishing block to the controller
                {
                    GlobalData.GcodeFileLines = null;
                    GlobalData.GcodeFileCurrentLine = 0;
                    WriteBuffer("M29\n");
                    WriteBuffer("M23 0.nc\n");
                    WriteBuffer("M24\n");
                    ui_widgets.setVisability("Gcode Upload", false);
                }
            }
        }
    }
}
