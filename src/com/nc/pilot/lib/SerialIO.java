/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.nc.pilot.lib;
import com.nc.pilot.lib.MotionController.MotionController;
import jssc.*;

import java.io.IOException;

public class SerialIO {

    MotionController motion_controller;
    SerialPort serialPort;
    String rx_line_buffer;
	public int open(String portName) {
        serialPort = new SerialPort(portName);
        try {
            serialPort.openPort();

            serialPort.setParams(SerialPort.BAUDRATE_9600,
                    SerialPort.DATABITS_8,
                    SerialPort.STOPBITS_1,
                    SerialPort.PARITY_NONE);

            serialPort.setFlowControlMode(SerialPort.FLOWCONTROL_RTSCTS_IN |
                    SerialPort.FLOWCONTROL_RTSCTS_OUT);

            serialPort.addEventListener(new PortReader(), SerialPort.MASK_RXCHAR);
            return 0;
        }
        catch (SerialPortException ex) {
            System.err.println("Could not open port т: " + ex);
            return -1;
        }
	}
	public int write(String buff)
    {
        try {
            serialPort.writeString(buff);
            return 0;
        } catch (SerialPortException e) {
            e.printStackTrace();
            return -1;
        }
    }
    public int writeByte(byte b)
    {
        try {
            serialPort.writeByte(b);
            return 0;
        } catch (SerialPortException e) {
            e.printStackTrace();
            return -1;
        }
    }
	public String[] get_available_ports()
    {
        String[] portNames = SerialPortList.getPortNames();

        if (portNames.length == 0) {
            try {
                System.in.read();
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
            return portNames;
        }
        String[] ret = new String[1];
        ret[0] = "None";
        return ret;
    }
    public void inherit_motion_controller(MotionController m)
    {
        motion_controller = m;
    }
	public synchronized void close()
    {
        try {
            serialPort.closePort();
        } catch (SerialPortException e) {
            e.printStackTrace();
        }
    }
    private class PortReader implements SerialPortEventListener {

        @Override
        public void serialEvent(SerialPortEvent event) {
            if(event.isRXCHAR() && event.getEventValue() > 0) {
                try {
                    String receivedData = serialPort.readString(event.getEventValue());
                    //System.out.println("Received response: " + receivedData);
                    if (receivedData != null)
                    {
                        for (int x = 0; x < receivedData.length(); x++)
                        {
                            if (receivedData.charAt(x) == '\n')
                            {
                                if (rx_line_buffer != null)
                                {
                                    motion_controller.ReadBuffer(rx_line_buffer);
                                    //System.out.println("Read Line: " + rx_line_buffer);
                                    rx_line_buffer = "";
                                }
                            }
                            else
                            {
                                if (receivedData.charAt(x) != '\r')
                                {
                                    rx_line_buffer += receivedData.charAt(x);
                                }
                            }
                        }
                    }
                }
                catch (SerialPortException ex) {
                    System.out.println("Error in receiving string from COM-port: " + ex);
                }
            }
        }

    }
}