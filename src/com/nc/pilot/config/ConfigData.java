package com.nc.pilot.config;

import java.io.*;
import java.util.ArrayList;

/**
 * Created by admin on 3/17/19.
 */
public class ConfigData {
    public ArrayList<JetToolpathCutChartData> CutChart;

    //Static variables for Jet Toolpath class
    public String MaterialSelection = "";
    public String ConsumableSelection = "";

    public String JetToolpathJobFile;

    public String CurrentWorkbench = "";

    public String LastGcodeOpenDir = "";
}
