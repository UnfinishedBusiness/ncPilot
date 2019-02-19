/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.nc.pilot.lib;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.OutputStream;

import com.nc.pilot.lib.MotionController.MotionController;
import gnu.io.CommPortIdentifier;
import gnu.io.SerialPort;
import gnu.io.SerialPortEvent; 
import gnu.io.SerialPortEventListener; 
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.UnsupportedEncodingException;
import java.io.Writer;
import java.util.Enumeration;
import java.util.logging.Level;
import java.util.logging.Logger;


public class SerialIO implements SerialPortEventListener {
	SerialPort serialPort;
    long last_write;
    int write_wait = 50; //Don't write to port any faster that 100m/s between writes
    private MotionController motion_controller;
        /** The port we're normally going to use. */
	private static final String PORT_NAMES[] = { 
			"COM6",
                "/dev/tty.usbmodem1411",
                "/dev/ttyUSB1",
                "/dev/ttyUSB0"
	};
	/**
	* A BufferedReader which will be fed by a InputStreamReader 
	* converting the bytes into characters 
	* making the displayed results codepage independent
	*/
	private BufferedReader input;
	/** The output stream to the port */
	private OutputStream output;
	/** Milliseconds to block while waiting for port open */
	private static final int TIME_OUT = 2000;
	/** Default bits per second for COM port. */
	private static final int DATA_RATE = 115200;

	public void initialize() {
		CommPortIdentifier portId = null;
		Enumeration portEnum = CommPortIdentifier.getPortIdentifiers();

		//First, Find an instance of serial port as set in PORT_NAMES.
		while (portEnum.hasMoreElements()) {
			CommPortIdentifier currPortId = (CommPortIdentifier) portEnum.nextElement();
			for (String portName : PORT_NAMES) {
				if (currPortId.getName().equals(portName)) {
					portId = currPortId;
					break;
				}
			}
		}
		if (portId == null) {
			System.out.println("Could not find COM port.");
			return;
		}

		try {
			// open serial port, and use class name for the appName.
			serialPort = (SerialPort) portId.open(this.getClass().getName(),
					TIME_OUT);

			// set port parameters
			serialPort.setSerialPortParams(DATA_RATE,
					SerialPort.DATABITS_8,
					SerialPort.STOPBITS_1,
					SerialPort.PARITY_NONE);

			// open the streams
			input = new BufferedReader(new InputStreamReader(serialPort.getInputStream()));
			output = serialPort.getOutputStream();

			// add event listeners
			serialPort.addEventListener(this);
			serialPort.notifyOnDataAvailable(true);
		} catch (Exception e) {
			System.err.println(e.toString());
		}
	}
    public void inherit_motion_controller(MotionController m)
    {
        motion_controller = m;
    }
	/**
	 * This should be called when you stop using the port.
	 * This will prevent port locking on platforms like Linux.
	 */
	public synchronized void close() {
		if (serialPort != null) {
			serialPort.removeEventListener();
			serialPort.close();
		}
	}
        public synchronized void writeByte(byte b)
        {
             if (serialPort != null){
                 try {
                     output.write(b);
                 } catch (IOException ex) {
                     Logger.getLogger(SerialIO.class.getName()).log(Level.SEVERE, null, ex);
                 }
            }
        }
        public synchronized void write(String data)
        {
            System.out.println("Writing " + data);
             if (serialPort != null){
                     Writer w = null;
                 try {
                     w = new OutputStreamWriter(output, "Cp1252");
                         try {
                             /*long current_write = System.currentTimeMillis();
                             if (current_write > last_write + write_wait)
                             {
                                 //System.out.println("Write Succesions are more that 100ms in between!");
                             }
                             else
                             {
                                 //System.out.println("Write Succesions are less than 100ms in between! waiting");
                                 try {
                                     Thread.sleep(write_wait);
                                 } catch (InterruptedException ex) {
                                     Thread.currentThread().interrupt();
                                 }
                             }*/
                             w.write(data);
                             last_write = System.currentTimeMillis();

                         } catch (IOException ex) {
                             Logger.getLogger(SerialIO.class.getName()).log(Level.SEVERE, null, ex);
                         }
                         try {
                             w.close(); //close will auto-flush
                         } catch (IOException ex) {
                             Logger.getLogger(SerialIO.class.getName()).log(Level.SEVERE, null, ex);
                         }
                 } catch (UnsupportedEncodingException ex) {
                     Logger.getLogger(SerialIO.class.getName()).log(Level.SEVERE, null, ex);
                 } finally {
                     try {
                         w.close();
                     } catch (IOException ex) {
                         Logger.getLogger(SerialIO.class.getName()).log(Level.SEVERE, null, ex);
                     }
                 }
            }
        }

	/**
	 * Handle an event on the serial port. Read the data and print it.
	 */
        public synchronized void serialEvent(SerialPortEvent oEvent) {
            if (oEvent.getEventType() == SerialPortEvent.DATA_AVAILABLE) {
                try {
                    String inputLine=input.readLine();
                    motion_controller.ReadBuffer(inputLine);

                } catch (Exception e) {
                    //System.err.println(e.toString());
                }
            }
        // Ignore all the other eventTypes, but you should consider the other ones.
    }
}