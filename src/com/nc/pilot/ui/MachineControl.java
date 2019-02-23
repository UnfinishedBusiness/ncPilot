package com.nc.pilot.ui;

import com.nc.pilot.lib.*;
import com.nc.pilot.lib.MDIConsole.MDIConsole;
import com.nc.pilot.lib.MotionController.MotionController;
import com.nc.pilot.lib.MotionController.MotionEngine;
import com.nc.pilot.lib.UIWidgets.UIWidgets;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.io.File;
import java.util.ArrayList;
import java.util.Timer;
import java.util.TimerTask;

/**
 * This program demonstrates how to draw lines using Graphics2D object.
 * @author www.codejava.net
 *
 */
public class MachineControl extends JFrame {

    private SerialIO serial;
    Timer repaint_timer = new Timer();
    Timer motion_engine_timer = new Timer();
    MotionController motion_controller;
    UIWidgets ui_widgets;
    GcodeViewer gcode_viewer;
    MDIConsole mdi_console;
    MotionEngine motion_engine;
    public MachineControl() {

        super("Xmotion Gen3 - Machine Control");
        setSize(1100, 800);
        setExtendedState(JFrame.MAXIMIZED_BOTH);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setLocationRelativeTo(null);
        serial = new SerialIO();
        serial.initialize();
        motion_controller = new MotionController(serial);
        serial.inherit_motion_controller(motion_controller);
        motion_controller.InitMotionController();
        ui_widgets = new UIWidgets();
        gcode_viewer = new GcodeViewer();
        mdi_console = new MDIConsole();
        motion_controller.inherit_ui_widgets(ui_widgets);
        motion_controller.inherit_mdi_console(mdi_console);
        motion_engine = new MotionEngine();
        Layout_UI();
        GcodeViewerPanel panel = new GcodeViewerPanel();
        add(panel);

        repaint_timer.schedule(new TimerTask() {
            @Override
            public void run() {
                repaint();
            }
        }, 0, 50);
        motion_engine_timer.schedule(new TimerTask() {
            @Override
            public void run() {
                motion_engine.Poll();
            }
        }, 0, 5);
        panel.addMouseListener(new MouseAdapter() {
            @Override
            public void mousePressed(MouseEvent e) {
                //System.out.println(e.getX() + "," + e.getY());
                ui_widgets.ClickPressStack(e.getX(), e.getY());
                repaint();
            }

            public void mouseReleased(MouseEvent e) {
                //System.out.println(e.getX() + "," + e.getY());
                ui_widgets.ClickReleaseStack(e.getX(), e.getY());
                repaint();
            }
        });
        panel.addMouseMotionListener(new MouseAdapter() {// provides empty implementation of all
            @Override
            public void mouseMoved(MouseEvent e) {
                //System.out.println(e.getX() + "," + e.getY());
                GlobalData.MousePositionX = e.getX();
                GlobalData.MousePositionY = e.getY();
                ui_widgets.MouseMotionStack(e.getX(), e.getY());
            }

            public void mouseDragged(MouseEvent e) {
                //System.out.println(e.getX() + "," + e.getY());
                GlobalData.MousePositionX = e.getX();
                GlobalData.MousePositionY = e.getY();
                ui_widgets.MouseMotionStack(e.getX(), e.getY());
                repaint();
            }
        });
        panel.addMouseWheelListener(new MouseWheelListener() {
            @Override
            public void mouseWheelMoved(MouseWheelEvent e) {
                float old_zoom = GlobalData.ViewerZoom;
                if (e.getWheelRotation() < 0) {
                    GlobalData.ViewerZoom *= 1.2;
                    //System.out.println("ViewerZoom: " + GlobalData.ViewerZoom);
                    if (GlobalData.ViewerZoom > GlobalData.MaxViewerZoom) {
                        GlobalData.ViewerZoom = GlobalData.MaxViewerZoom;
                    }
                } else {
                    GlobalData.ViewerZoom *= 0.8;
                    //System.out.println("ViewerZoom: " + GlobalData.ViewerZoom);
                    if (GlobalData.ViewerZoom < GlobalData.MinViewerZoom) {
                        GlobalData.ViewerZoom = GlobalData.MinViewerZoom;
                    }
                }
                float scalechange = GlobalData.ViewerZoom - old_zoom;
                //printf("Scale change: %0.4f\n", scalechange);
                float pan_x = (GlobalData.MousePositionX_MCS * scalechange) * -1;
                float pan_y = (GlobalData.MousePositionY_MCS * scalechange);
                //System.out.println("Pan_x: " + pan_x + " Pan_y: " + pan_y);
                GlobalData.ViewerPan[0] += pan_x;
                GlobalData.ViewerPan[1] += pan_y;
                repaint();
            }
        });

        KeyboardFocusManager.getCurrentKeyboardFocusManager()
                .addKeyEventDispatcher(new KeyEventDispatcher() {
                    @Override
                    public boolean dispatchKeyEvent(KeyEvent ke) {
                        switch (ke.getID()) {
                            case KeyEvent.KEY_PRESSED:
                                if (ke.getKeyCode() == KeyEvent.VK_UP) {
                                    if (GlobalData.UpArrowKeyState == false) {
                                        GlobalData.UpArrowKeyState = true;
                                        MotionController.JogY_Plus();
                                    }
                                }
                                if (ke.getKeyCode() == KeyEvent.VK_DOWN) {
                                    if (GlobalData.DownArrowKeyState == false) {
                                        GlobalData.DownArrowKeyState = true;
                                        MotionController.JogY_Minus();
                                    }
                                }
                                if (ke.getKeyCode() == KeyEvent.VK_RIGHT) {
                                    if (GlobalData.RightArrowKeyState == false) {
                                        GlobalData.RightArrowKeyState = true;
                                        MotionController.JogX_Plus();
                                    }
                                }
                                if (ke.getKeyCode() == KeyEvent.VK_LEFT) {
                                    if (GlobalData.LeftArrowKeyState == false) {
                                        GlobalData.LeftArrowKeyState = true;
                                        MotionController.JogX_Minus();
                                    }
                                }
                                if (ke.getKeyCode() == KeyEvent.VK_PAGE_UP) {
                                    if (GlobalData.PageUpKeyState == false) {
                                        GlobalData.PageUpKeyState = true;
                                        MotionController.JogZ_Plus();
                                    }
                                }
                                if (ke.getKeyCode() == KeyEvent.VK_PAGE_DOWN) {
                                    if (GlobalData.PageDownKeyState == false) {
                                        GlobalData.PageDownKeyState = true;
                                        MotionController.JogZ_Minus();
                                    }
                                }
                                if (ke.getKeyCode() == KeyEvent.VK_ALT) {

                                    GlobalData.AltKeyPressed = true;
                                }
                                //repaint();
                                break;

                            case KeyEvent.KEY_RELEASED:
                                if (ke.getKeyCode() == KeyEvent.VK_UP) {
                                    GlobalData.UpArrowKeyState = false;
                                    MotionController.EndJog();

                                }
                                if (ke.getKeyCode() == KeyEvent.VK_DOWN) {
                                    GlobalData.DownArrowKeyState = false;
                                    MotionController.EndJog();
                                }
                                if (ke.getKeyCode() == KeyEvent.VK_LEFT) {
                                    GlobalData.LeftArrowKeyState = false;
                                    MotionController.EndJog();
                                }
                                if (ke.getKeyCode() == KeyEvent.VK_RIGHT) {
                                    GlobalData.RightArrowKeyState = false;
                                    MotionController.EndJog();
                                }
                                if (ke.getKeyCode() == KeyEvent.VK_PAGE_UP) {
                                    GlobalData.PageUpKeyState = false;
                                    MotionController.EndJog();
                                }
                                if (ke.getKeyCode() == KeyEvent.VK_PAGE_DOWN) {
                                    GlobalData.PageDownKeyState = false;
                                    MotionController.EndJog();
                                }
                                if (ke.getKeyCode() == KeyEvent.VK_SPACE) {

                                    MotionController.FeedHold();
                                }
                                if (ke.getKeyCode() == KeyEvent.VK_TAB)
                                {
                                    GcodeInterpreter g = new GcodeInterpreter("/home/cnc/Desktop/Post/0.ngc");
                                    GlobalData.GcodeFile = "/home/cnc/Desktop/Post/0.ngc";
                                    ArrayList<GcodeInterpreter.GcodeMove> moves = g.GetMoves();
                                    gcode_viewer.ClearStack();
                                    for (int x = 2; x < moves.size(); x ++)
                                    {
                                        if (moves.get(x).Gword == 1)
                                        {
                                            gcode_viewer.addLine(new float[]{moves.get(x-1).Xword, moves.get(x-1).Yword}, new float[]{moves.get(x).Xword, moves.get(x).Yword});
                                        }
                                        if (moves.get(x).Gword == 2)
                                        {
                                            float[] center = new float[]{moves.get(x-1).Xword + moves.get(x).Iword, moves.get(x-1).Yword + moves.get(x).Jword};
                                            float radius = new Float(Math.hypot(moves.get(x).Xword-center[0], moves.get(x).Yword-center[1]));
                                            gcode_viewer.addArc(new float[]{moves.get(x-1).Xword, moves.get(x-1).Yword}, new float[]{moves.get(x).Xword, moves.get(x).Yword}, center, radius, "CW");
                                        }
                                        if (moves.get(x).Gword == 3)
                                        {
                                            float[] center = new float[]{moves.get(x-1).Xword + moves.get(x).Iword, moves.get(x-1).Yword + moves.get(x).Jword};
                                            float radius = new Float(Math.hypot(moves.get(x).Xword-center[0], moves.get(x).Yword-center[1]));
                                            gcode_viewer.addArc(new float[]{moves.get(x-1).Xword, moves.get(x-1).Yword}, new float[]{moves.get(x).Xword, moves.get(x).Yword}, center, radius, "CCW");
                                        }
                                    }
                                }
                                if (ke.getKeyCode() == KeyEvent.VK_ESCAPE) {
                                    MotionController.FeedHold();
                                    MotionController.Abort();
                                }
                                if (ke.getKeyCode() == KeyEvent.VK_R) {

                                    /*if (GlobalData.AltKeyPressed == true)
                                    {
                                        motion_controller.CycleStart();
                                    }*/
                                    motion_engine.runMoves();
                                }
                                if (ke.getKeyCode() == KeyEvent.VK_F1) {

                                    if (GlobalData.AltKeyPressed == true)
                                    {
                                        mdi_console.show();
                                    }
                                }
                                if (ke.getKeyCode() == KeyEvent.VK_ALT) {

                                    GlobalData.AltKeyPressed = false;
                                }
                                break;
                        }
                        mdi_console.dispatchKeyEvent(ke);
                        return false;
                    }
                });
    }
    private void Layout_UI()
    {
        ui_widgets.AddDRO();
        ui_widgets.AddMomentaryButton("Open", "bottom-right", 80, 60, 10, 10, new Runnable() {
            @Override
            public void run() {
                JFileChooser fileChooser = new JFileChooser();
                fileChooser.setCurrentDirectory(new File("."));
                int result = fileChooser.showOpenDialog(getParent());
                if (result == JFileChooser.APPROVE_OPTION) {
                    File selectedFile = fileChooser.getSelectedFile();
                    System.out.println("Selected file: " + selectedFile.getAbsolutePath());
                    GcodeInterpreter g = new GcodeInterpreter(selectedFile.getAbsolutePath());
                    GlobalData.GcodeFile = selectedFile.getAbsolutePath();
                    ArrayList<GcodeInterpreter.GcodeMove> moves = g.GetMoves();
                    motion_engine.setMoves(moves);
                    gcode_viewer.ClearStack();

                    for (int x = 2; x < moves.size(); x ++)
                    {
                        if (moves.get(x).Gword == 1)
                        {
                            gcode_viewer.addLine(new float[]{moves.get(x-1).Xword, moves.get(x-1).Yword}, new float[]{moves.get(x).Xword, moves.get(x).Yword});
                        }
                        if (moves.get(x).Gword == 2)
                        {
                            float[] center = new float[]{moves.get(x-1).Xword + moves.get(x).Iword, moves.get(x-1).Yword + moves.get(x).Jword};
                            float radius = new Float(Math.hypot(moves.get(x).Xword-center[0], moves.get(x).Yword-center[1]));
                            gcode_viewer.addArc(new float[]{moves.get(x-1).Xword, moves.get(x-1).Yword}, new float[]{moves.get(x).Xword, moves.get(x).Yword}, center, radius, "CW");
                        }
                        if (moves.get(x).Gword == 3)
                        {
                            float[] center = new float[]{moves.get(x-1).Xword + moves.get(x).Iword, moves.get(x-1).Yword + moves.get(x).Jword};
                            float radius = new Float(Math.hypot(moves.get(x).Xword-center[0], moves.get(x).Yword-center[1]));
                            gcode_viewer.addArc(new float[]{moves.get(x-1).Xword, moves.get(x-1).Yword}, new float[]{moves.get(x).Xword, moves.get(x).Yword}, center, radius, "CCW");
                        }
                    }
                }
            }
        });
        ui_widgets.AddMomentaryButton("Abort", "bottom-right", 80, 60, 100, 10, new Runnable() {
            @Override
            public void run() {
                System.out.println("Clicked on Abort!");
                MotionController.FeedHold();
                MotionController.Abort();
            }
        });
        ui_widgets.AddMomentaryButton("Hold", "bottom-right", 80, 60, 190, 10, new Runnable() {
            @Override
            public void run() {
                System.out.println("Clicked on Hold!");
                MotionController.FeedHold();
            }
        });
        ui_widgets.AddMomentaryButton("Start", "bottom-right", 80, 60, 280, 10, new Runnable() {
            @Override
            public void run() {
                System.out.println("Clicked on Start!");
                MotionController.CycleStart();
            }
        });
        ui_widgets.AddSelectButton("Torch Off","torch", true, "bottom-right", 170, 60, 10, 80, new Runnable() {
            @Override
            public void run() {
                System.out.println("Torch Off!");
            }
        });
        ui_widgets.AddSelectButton("Torch On", "torch", false,"bottom-right", 170, 60, 190, 80, new Runnable() {
            @Override
            public void run() {
                System.out.println("Torch On!");
            }
        });
        ui_widgets.AddMomentaryButton("Go Home", "bottom-right", 120, 60, 10, 150, new Runnable() {
            @Override
            public void run() {
                System.out.println("Go Home!");
                motion_controller.WriteBuffer("G90 G53 G0 X0 Y0\n");
            }
        });
        ui_widgets.AddMomentaryButton("Probe Z", "bottom-right", 120, 60, 140, 150, new Runnable() {
            @Override
            public void run() {
                System.out.println("Probe Z!");
            }
        });
        ui_widgets.AddMomentaryButton("Home", "bottom-right", 90, 60, 270, 150, new Runnable() {
            @Override
            public void run() {
                System.out.println("Home!");
                if (GlobalData.IsHomed == false)
                {
                    GlobalData.IsHomed = true;
                    motion_controller.Home();
                }
            }
        });
        ui_widgets.AddMomentaryButton("X=0", "bottom-right", 110, 60, 250, 220, new Runnable() {
            @Override
            public void run() {
                System.out.println("X=0");
                motion_controller.SetXzero();
            }
        });
        ui_widgets.AddMomentaryButton("Y=0", "bottom-right", 110, 60, 130, 220, new Runnable() {
            @Override
            public void run() {
                System.out.println("Y=0");
                motion_controller.SetYzero();
            }
        });
        ui_widgets.AddMomentaryButton("Z=0", "bottom-right", 110, 60, 10, 220, new Runnable() {
            @Override
            public void run() {
                System.out.println("Z=0");
                motion_controller.SetZzero();
            }
        });
        ui_widgets.AddSelectButton("0.001\"", "jog", false,"bottom-right", 60, 60, 10, 290, new Runnable() {
            @Override
            public void run() {
                //System.out.println("Clicked on 0.001\"!");
                GlobalData.JogMode = "0.001";
            }
        });
        ui_widgets.AddSelectButton("0.01\"","jog", false, "bottom-right", 60, 60, 80, 290, new Runnable() {
            @Override
            public void run() {
                //System.out.println("Clicked on 0.01\"!");
                GlobalData.JogMode = "0.01";
            }
        });
        ui_widgets.AddSelectButton("0.1\"", "jog", false,"bottom-right", 60, 60, 150, 290, new Runnable() {
            @Override
            public void run() {
                //System.out.println("Clicked on 0.1\"");
                GlobalData.JogMode = "0.1";
            }
        });
        ui_widgets.AddSelectButton("Continuous","jog",true, "bottom-right", 140, 60, 220, 290, new Runnable() {
            @Override
            public void run() {
                //System.out.println("Clicked on Continuous!");
                GlobalData.JogMode = "Continuous";
            }
        });
        //void AddSlider(String text, String anchor, int width, int height, int posx, int posy, int min, int max, Runnable action){
        /*ui_widgets.AddSlider("Jog Speed", "bottom-right", 350, 60, 10, 360, 0, (int)GlobalData.X_Max_Vel , (int)(GlobalData.X_Max_Vel * 0.7), new Runnable(){
            @Override
            public void run() {
                System.out.println("New position: " + ui_widgets.getSliderPosition("Jog Speed"));
            }
        });*/
        ui_widgets.AddSlider("Jog Speed", "bottom-right", 350, 60, 10, 360, 0, (int)GlobalData.Max_linear_Vel, 300, "Inch/Min", new Runnable(){
            @Override
            public void run() {
                System.out.println("New position: " + ui_widgets.getSliderPosition("Jog Speed"));
                motion_controller.SetJogSpeed(ui_widgets.getSliderPosition("Jog Speed"));
            }
        });
    }
    // create a panel that you can draw on.
    class GcodeViewerPanel extends JPanel {
        public void paint(Graphics g) {
            Rectangle Frame_Bounds = this.getParent().getBounds();
            GlobalData.MousePositionX_MCS = (GlobalData.MousePositionX - GlobalData.ViewerPan[0]) / GlobalData.ViewerZoom;
            GlobalData.MousePositionY_MCS = ((GlobalData.MousePositionY - GlobalData.ViewerPan[1]) / GlobalData.ViewerZoom) * -1;
            Graphics2D g2d = (Graphics2D) g;
            /* Begin Wallpaper */
            g.setColor(Color.black);
            g.fillRect(0,0,Frame_Bounds.width,Frame_Bounds.height);
            /* End Wallpaper */
            gcode_viewer.RenderStack(g2d);
            ui_widgets.RenderStack(g2d, Frame_Bounds);
            mdi_console.RenderStack(g2d, Frame_Bounds);
            //Display Mouse position in MCS and Screen Cord
            //g.setColor(Color.green);
            //g.setFont(new Font("Arial", Font.BOLD, 12));
            //g.drawString(String.format("Screen-> X: %d Y: %d MCS-> X: %.4f Y: %.4f", GlobalData.MousePositionX, GlobalData.MousePositionY, GlobalData.MousePositionX_MCS, GlobalData.MousePositionY_MCS), 10, 10);
        }
    }
    public static void main(String[] args) {
        SwingUtilities.invokeLater(new Runnable() {
            @Override
            public void run() {
                new MachineControl().setVisible(true);
            }
        });
    }
}