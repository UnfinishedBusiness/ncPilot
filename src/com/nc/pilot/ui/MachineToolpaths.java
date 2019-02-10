package com.nc.pilot.ui;

import com.nc.pilot.lib.*;

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
public class MachineToolpaths extends JFrame {

    UIWidgets ui_widgets;
    ToolpathViewer toolpath_viewer;
    public MachineToolpaths() {

        super("Xmotion Gen3 - Machine Toolpaths");
        setSize(1100, 800);
        setExtendedState(JFrame.MAXIMIZED_BOTH);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setLocationRelativeTo(null);

        //motion_controller.InitMotionController();
        ui_widgets = new UIWidgets();
        toolpath_viewer = new ToolpathViewer();
        
        Layout_UI();
        ToolpathViewerPanel panel = new ToolpathViewerPanel();
        add(panel);
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
                repaint();
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
            public void mouseWheelMoved(MouseWheelEvent e)
            {
                float old_zoom = GlobalData.ViewerZoom;
                if (e.getWheelRotation() < 0) {
                    GlobalData.ViewerZoom *= 1.2;
                    //System.out.println("ViewerZoom: " + GlobalData.ViewerZoom);
                    if (GlobalData.ViewerZoom > GlobalData.MaxViewerZoom)
                    {
                        GlobalData.ViewerZoom = GlobalData.MaxViewerZoom;
                    }
                } else {
                    GlobalData.ViewerZoom *= 0.8;
                    //System.out.println("ViewerZoom: " + GlobalData.ViewerZoom);
                    if (GlobalData.ViewerZoom < GlobalData.MinViewerZoom)
                    {
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
                                    if (GlobalData.UpArrowKeyState == false)
                                    {
                                        GlobalData.UpArrowKeyState = true;
                                        MotionController.JogY_Plus();
                                    }
                                }
                                if (ke.getKeyCode() == KeyEvent.VK_DOWN) {
                                    if (GlobalData.DownArrowKeyState == false)
                                    {
                                        GlobalData.DownArrowKeyState = true;
                                        MotionController.JogY_Minus();
                                    }
                                }
                                if (ke.getKeyCode() == KeyEvent.VK_RIGHT) {
                                    if (GlobalData.RightArrowKeyState == false)
                                    {
                                        GlobalData.RightArrowKeyState = true;
                                        MotionController.JogX_Plus();
                                    }
                                }
                                if (ke.getKeyCode() == KeyEvent.VK_LEFT) {
                                    if (GlobalData.LeftArrowKeyState == false)
                                    {
                                        GlobalData.LeftArrowKeyState = true;
                                        MotionController.JogX_Minus();
                                    }
                                }
                                if (ke.getKeyCode() == KeyEvent.VK_PAGE_UP) {
                                    if (GlobalData.RightArrowKeyState == false)
                                    {
                                        GlobalData.PageUpKeyState = true;
                                        MotionController.JogZ_Plus();
                                    }
                                }
                                if (ke.getKeyCode() == KeyEvent.VK_PAGE_DOWN) {
                                    if (GlobalData.LeftArrowKeyState == false)
                                    {
                                        GlobalData.PageDownKeyState = true;
                                        MotionController.JogZ_Minus();
                                    }
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

                                    toolpath_viewer.setJobMaterial(20, 20);
                                    repaint();
                                }

                                break;
                        }
                        return false;
                    }
                });
    }
    private void Layout_UI()
    {
        ui_widgets.AddMomentaryButton("Open", "bottom-right", 80, 60, 10, 10, new Runnable() {
            @Override
            public void run() {
                JFileChooser fileChooser = new JFileChooser();
                fileChooser.setCurrentDirectory(new File(System.getProperty("user.home")));
                int result = fileChooser.showOpenDialog(getParent());
                if (result == JFileChooser.APPROVE_OPTION) {
                    File selectedFile = fileChooser.getSelectedFile();
                    System.out.println("Selected file: " + selectedFile.getAbsolutePath());
                    GcodeInterpreter g = new GcodeInterpreter(selectedFile.getAbsolutePath());
                    ArrayList<GcodeInterpreter.GcodeMove> moves = g.GetMoves();
                    toolpath_viewer.ClearStack();

                    for (int x = 2; x < moves.size(); x ++)
                    {
                        if (moves.get(x).Gword == 1)
                        {
                            toolpath_viewer.addLine(new float[]{moves.get(x-1).Xword, moves.get(x-1).Yword}, new float[]{moves.get(x).Xword, moves.get(x).Yword});
                        }
                        if (moves.get(x).Gword == 2)
                        {
                            float[] center = new float[]{moves.get(x-1).Xword + moves.get(x).Iword, moves.get(x-1).Yword + moves.get(x).Jword};
                            float radius = new Float(Math.hypot(moves.get(x).Xword-center[0], moves.get(x).Yword-center[1]));
                            toolpath_viewer.addArc(new float[]{moves.get(x-1).Xword, moves.get(x-1).Yword}, new float[]{moves.get(x).Xword, moves.get(x).Yword}, center, radius, "CW");
                        }
                        if (moves.get(x).Gword == 3)
                        {
                            float[] center = new float[]{moves.get(x-1).Xword + moves.get(x).Iword, moves.get(x-1).Yword + moves.get(x).Jword};
                            float radius = new Float(Math.hypot(moves.get(x).Xword-center[0], moves.get(x).Yword-center[1]));
                            toolpath_viewer.addArc(new float[]{moves.get(x-1).Xword, moves.get(x-1).Yword}, new float[]{moves.get(x).Xword, moves.get(x).Yword}, center, radius, "CCW");
                        }
                    }
                }
            }
        });
    }
    // create a panel that you can draw on.
    class ToolpathViewerPanel extends JPanel {
        public void paint(Graphics g) {
            Rectangle Frame_Bounds = this.getParent().getBounds();
            GlobalData.MousePositionX_MCS = (GlobalData.MousePositionX - GlobalData.ViewerPan[0]) / GlobalData.ViewerZoom;
            GlobalData.MousePositionY_MCS = ((GlobalData.MousePositionY - GlobalData.ViewerPan[1]) / GlobalData.ViewerZoom) * -1;
            Graphics2D g2d = (Graphics2D) g;
            /* Begin Wallpaper */
            g.setColor(Color.black);
            g.fillRect(0,0,Frame_Bounds.width,Frame_Bounds.height);
            /* End Wallpaper */
            toolpath_viewer.RenderStack(g2d);
            ui_widgets.RenderStack(g2d, Frame_Bounds);
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
                new MachineToolpaths().setVisible(true);
            }
        });
    }
}