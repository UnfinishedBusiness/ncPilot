package com.nc.pilot.ui;

import com.nc.pilot.dialogs.JetToolpathCutChart;
import com.nc.pilot.dialogs.JetToolpathJobSetup;
import com.nc.pilot.lib.*;
import com.nc.pilot.lib.ToolPathViewer.ToolpathViewer;
import com.nc.pilot.lib.UIWidgets.UIWidgets;
import org.kabeja.parser.ParseException;

import javax.swing.*;
import javax.swing.filechooser.FileFilter;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.util.ArrayList;

public class JetToolpaths extends JFrame {
    JMenuBar menu_bar;
    UIWidgets ui_widgets;
    ToolpathViewer toolpath_viewer;


    public JetToolpaths() {

        super("Jet Toolpaths");
        GlobalData.configData.CurrentWorkbench = "JetToolpaths";
        setSize(1100, 800);
        setExtendedState(JFrame.MAXIMIZED_BOTH);
        setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
        setLocationRelativeTo(null);

        ui_widgets = new UIWidgets();
        toolpath_viewer = new ToolpathViewer();
        
        Layout_UI();
        createMenuBar();
        setJMenuBar(menu_bar);
        ToolpathViewerPanel panel = new ToolpathViewerPanel();
        add(panel);
        panel.addMouseListener(new MouseAdapter() {
            @Override
            public void mousePressed(MouseEvent e) {
                //System.out.println(e.getX() + "," + e.getY());
                ui_widgets.ClickPressStack(e.getX(), e.getY());
                GlobalData.MousePositionX_MCS = (GlobalData.MousePositionX - GlobalData.ViewerPan[0]) / GlobalData.ViewerZoom;
                GlobalData.MousePositionY_MCS = ((GlobalData.MousePositionY - GlobalData.ViewerPan[1]) / GlobalData.ViewerZoom) * -1;
                toolpath_viewer.ClickPressStack(GlobalData.MousePositionX_MCS, GlobalData.MousePositionY_MCS);
                repaint();
            }
            public void mouseReleased(MouseEvent e) {
                //System.out.println(e.getX() + "," + e.getY());
                ui_widgets.ClickReleaseStack(e.getX(), e.getY());
                GlobalData.MousePositionX_MCS = (GlobalData.MousePositionX - GlobalData.ViewerPan[0]) / GlobalData.ViewerZoom;
                GlobalData.MousePositionY_MCS = ((GlobalData.MousePositionY - GlobalData.ViewerPan[1]) / GlobalData.ViewerZoom) * -1;
                toolpath_viewer.ClickReleaseStack(GlobalData.MousePositionX_MCS, GlobalData.MousePositionY_MCS);
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
                GlobalData.MousePositionX_MCS = (GlobalData.MousePositionX - GlobalData.ViewerPan[0]) / GlobalData.ViewerZoom;
                GlobalData.MousePositionY_MCS = ((GlobalData.MousePositionY - GlobalData.ViewerPan[1]) / GlobalData.ViewerZoom) * -1;
                toolpath_viewer.MouseMotionStack(GlobalData.MousePositionX_MCS, GlobalData.MousePositionY_MCS);
                repaint();
            }
            public void mouseDragged(MouseEvent e) {
                //System.out.println(e.getX() + "," + e.getY());
                GlobalData.MousePositionX = e.getX();
                GlobalData.MousePositionY = e.getY();
                ui_widgets.MouseMotionStack(e.getX(), e.getY());
                GlobalData.MousePositionX_MCS = (GlobalData.MousePositionX - GlobalData.ViewerPan[0]) / GlobalData.ViewerZoom;
                GlobalData.MousePositionY_MCS = ((GlobalData.MousePositionY - GlobalData.ViewerPan[1]) / GlobalData.ViewerZoom) * -1;
                toolpath_viewer.MouseMotionStack(GlobalData.MousePositionX_MCS, GlobalData.MousePositionY_MCS);
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
                        if (!GlobalData.configData.CurrentWorkbench.contentEquals("JetToolpaths")) return false;
                        switch (ke.getID()) {
                            case KeyEvent.KEY_PRESSED:
                                //System.out.println("(Jet Toolpath) Key: " + ke.getKeyCode());
                                if (ke.getKeyCode() == 44) //< key
                                {
                                    toolpath_viewer.RotateEngagedPart(5);
                                    repaint();
                                }
                                if (ke.getKeyCode() == 46) //> key
                                {
                                    toolpath_viewer.RotateEngagedPart(-5);
                                    repaint();
                                }

                                //repaint();
                                break;

                            case KeyEvent.KEY_RELEASED:
                                if (ke.getKeyCode() == KeyEvent.VK_SPACE) {
                                    System.out.println("Toolpath Dry run!");
                                    toolpath_viewer.postProcess(null);
                                    repaint();
                                }
                                break;
                        }
                        return false;
                    }
                });

        // call onCancel() when cross is clicked
        setDefaultCloseOperation(DO_NOTHING_ON_CLOSE);
        addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent e) {
                onClose();
            }
        });

        if (GlobalData.configData.JetToolpathJobFile != null)
        {
            File f = new File(GlobalData.configData.JetToolpathJobFile);
            if(f.exists() && !f.isDirectory()) {
                toolpath_viewer.OpenJob(GlobalData.configData.JetToolpathJobFile);
            }
        }
    }
    private void onClose() {
        // add your code here if necessary
        GlobalData.configData.CurrentWorkbench = "MachineControl";
        dispose();
    }
    private void createMenuBar()
    {
        //Where the GUI is created:
        JMenu menu;
        JMenuItem menuItem;

        //Create the menu bar.
        menu_bar = new JMenuBar();

        //Build File menu
        menu = new JMenu("File");
        menu.setMnemonic(KeyEvent.VK_S);
        menu.getAccessibleContext().setAccessibleDescription("File operations");
        menu_bar.add(menu);

        menuItem = new JMenuItem("New Job");
        menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_0, ActionEvent.ALT_MASK));
        menuItem.getAccessibleContext().setAccessibleDescription("New Job File");
        menuItem.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                GlobalData.configData.JetToolpathJobFile = "";
                toolpath_viewer.ViewerPartStack = new ArrayList();
                try {
                    GlobalData.pushConfig();
                } catch (FileNotFoundException e1) {
                    e1.printStackTrace();
                }
                repaint();
            }
        });
        menu.add(menuItem);

        menuItem = new JMenuItem("Open Job");
        menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_1, ActionEvent.ALT_MASK));
        menuItem.getAccessibleContext().setAccessibleDescription("Open Job File");
        menuItem.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                JFileChooser fileChooser = new JFileChooser();
                fileChooser.setCurrentDirectory(new File("."));
                int result = fileChooser.showOpenDialog(getParent());
                if (result == JFileChooser.APPROVE_OPTION) {
                    File selectedFile = fileChooser.getSelectedFile();
                    System.out.println("Selected file: " + selectedFile.getAbsolutePath());
                    GlobalData.configData.JetToolpathJobFile = selectedFile.getAbsolutePath();
                    toolpath_viewer.OpenJob(GlobalData.configData.JetToolpathJobFile);
                    repaint();
                }
            }
        });
        menu.add(menuItem);

        menuItem = new JMenuItem("Save Job");
        menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_2, ActionEvent.ALT_MASK));
        menuItem.getAccessibleContext().setAccessibleDescription("Save Job File");
        menuItem.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (GlobalData.configData.JetToolpathJobFile == null) {
                    JFileChooser fileChooser = new JFileChooser();
                    fileChooser.setCurrentDirectory(new File("."));
                    fileChooser.addChoosableFileFilter(new FileFilter() {

                        public String getDescription() {
                            return "Xmotion Job File (*.xmj)";
                        }

                        public boolean accept(File f) {
                            if (f.isDirectory()) {
                                return true;
                            } else {
                                return f.getName().toLowerCase().endsWith(".xmj");
                            }
                        }
                    });
                    if (fileChooser.showSaveDialog(getParent()) == JFileChooser.APPROVE_OPTION) {
                        File file = fileChooser.getSelectedFile();
                        GlobalData.configData.JetToolpathJobFile = file.getAbsolutePath();
                        toolpath_viewer.SaveJob(GlobalData.configData.JetToolpathJobFile);
                        try {
                            GlobalData.pushConfig();
                        } catch (FileNotFoundException e1) {
                            e1.printStackTrace();
                        }
                    }
                }
                else
                {
                    toolpath_viewer.SaveJob(GlobalData.configData.JetToolpathJobFile);
                }
            }
        });
        menu.add(menuItem);

        menuItem = new JMenuItem("Save Job As");
        menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_3, ActionEvent.ALT_MASK));
        menuItem.getAccessibleContext().setAccessibleDescription("Save Job File As");
        menuItem.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                JFileChooser fileChooser = new JFileChooser();
                fileChooser.setCurrentDirectory(new File("."));
                fileChooser.addChoosableFileFilter(new FileFilter() {

                    public String getDescription() {
                        return "Xmotion Job File (*.xmj)";
                    }

                    public boolean accept(File f) {
                        if (f.isDirectory()) {
                            return true;
                        } else {
                            return f.getName().toLowerCase().endsWith(".xmj");
                        }
                    }
                });
                if (fileChooser.showSaveDialog(getParent()) == JFileChooser.APPROVE_OPTION) {
                    File file = fileChooser.getSelectedFile();
                    GlobalData.configData.JetToolpathJobFile = file.getAbsolutePath();
                    toolpath_viewer.SaveJob(GlobalData.configData.JetToolpathJobFile);
                }
            }
        });
        menu.add(menuItem);

        menuItem = new JMenuItem("Import Part");
        menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_4, ActionEvent.ALT_MASK));
        menuItem.getAccessibleContext().setAccessibleDescription("Import Part Drawing");
        menuItem.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                JFileChooser fileChooser = new JFileChooser();
                fileChooser.setCurrentDirectory(new File("."));
                int result = fileChooser.showOpenDialog(getParent());
                if (result == JFileChooser.APPROVE_OPTION) {
                    File selectedFile = fileChooser.getSelectedFile();
                    System.out.println("Selected file: " + selectedFile.getAbsolutePath());
                    try {
                        toolpath_viewer.OpenDXFasPart(selectedFile.getAbsolutePath(), selectedFile.getAbsolutePath());
                    } catch (ParseException e1) {
                        e1.printStackTrace();
                    }
                }
            }
        });
        menu.add(menuItem);

        menuItem = new JMenuItem("Post Process");
        menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_5, ActionEvent.ALT_MASK));
        menuItem.getAccessibleContext().setAccessibleDescription("Post Job into Gcode");
        menu.add(menuItem);
        menuItem.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                JFileChooser fileChooser = new JFileChooser();
                fileChooser.setCurrentDirectory(new File("."));
                if (fileChooser.showSaveDialog(getParent()) == JFileChooser.APPROVE_OPTION) {
                    File file = fileChooser.getSelectedFile();
                    toolpath_viewer.postProcess(file.getAbsolutePath());
                    repaint();
                }
            }
        });

        //Build Setup menu
        menu = new JMenu("Setup");
        menu.setMnemonic(KeyEvent.VK_S);
        menu.getAccessibleContext().setAccessibleDescription("Job Setup Parameters");
        menu_bar.add(menu);

        menuItem = new JMenuItem("Cut Chart");
        menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_1, ActionEvent.ALT_MASK));
        menuItem.getAccessibleContext().setAccessibleDescription("Setup Cut Library");
        menu.add(menuItem);
        menuItem.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                JetToolpathCutChart.main(null);
            }
        });

        menuItem = new JMenuItem("Job Setup");
        menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_2, ActionEvent.ALT_MASK));
        menuItem.getAccessibleContext().setAccessibleDescription("Setup Job Parameters");
        menu.add(menuItem);
        menuItem.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                JetToolpathJobSetup.main(null);
            }
        });

    }
    private void Layout_UI()
    {

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
                new JetToolpaths().setVisible(true);
            }
        });
    }
}