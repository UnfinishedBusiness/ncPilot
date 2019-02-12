/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.nc.pilot.lib;

import java.awt.geom.Point2D;
import java.io.IOException;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;

/**
 *
 * @author travis
 */
public class GlobalData {

    public static float[] dro = {0, 0, 0};
    public static float[] last_dro = {0, 0, 0};
    public static float[] work_offset = {0, 0, 0};
    public static float[] machine_cordinates = {0, 0, 0};
    public static String MachineState = "";
    public static float CurrentVelocity;
    public static float ProgrammedFeedrate;
    public static String CurrentUnits = "Inch";
    public static boolean IsHomed = false;
    public static boolean PlannerReady = false;
    public static int FreeBuffers;
    //public static ArrayList<String> WriteBuffer = new ArrayList();

    public static float X_Scale = 635f; //Steps/Inch
    public static float Y_Scale = 635f; //Steps/Inch
    public static float Z_Scale = 635f; //Steps/Inch
    public static float X_Accel = 5f; //Inch/Sec^2
    public static float Y_Accel = 5f; //Inch/Sec^2
    public static float Z_Accel = 5f; //Inch/Sec^2
    public static float X_Max_Vel = 600f; //Inch/Min
    public static float Y_Max_Vel = 600f; //Inch/Min
    public static float Z_Max_Vel = 60f; //Inch/Min
    public static float Max_linear_Vel = 600f;
    public static float X_Extents = 48.250f;
    public static float Y_Extents = 45.500f;
    public static float Z_Extents = -4.000f;

    //Used to ignore Pressed repeats!
    public static Boolean UpArrowKeyState = false;
    public static Boolean DownArrowKeyState= false;
    public static Boolean LeftArrowKeyState = false;
    public static Boolean RightArrowKeyState = false;
    public static Boolean PageUpKeyState = false;
    public static Boolean PageDownKeyState = false;


    public static float ViewerZoom = 19f;
    public static float MinViewerZoom = 0.005f;
    public static float MaxViewerZoom = 1000000000;
    public static float[] ViewerPan = {438, 900};
    public static int MousePositionX;
    public static int MousePositionY;
    public static float MousePositionX_MCS;
    public static float MousePositionY_MCS;

    
    public static String readFile(String path) throws IOException 
    {
       byte[] encoded = Files.readAllBytes(Paths.get(path));
       return new String(encoded);
    }
}