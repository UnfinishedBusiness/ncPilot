package com.nc.pilot.ui;

import com.nc.pilot.dialogs.JetToolpathCutChart;
import com.nc.pilot.dialogs.JetToolpathJobSetup;
import com.nc.pilot.lib.GlobalData;
import com.nc.pilot.lib.JetCad.DrawingStack.RenderEngine;
import com.nc.pilot.lib.UIWidgets.UIWidgets;
import org.kabeja.parser.ParseException;

import javax.swing.*;
import javax.swing.filechooser.FileFilter;
import java.awt.*;
import java.awt.event.*;
import java.io.File;
import java.io.FileNotFoundException;
import java.util.ArrayList;

public class JetCad extends JFrame {
    JMenuBar menu_bar;
    UIWidgets ui_widgets;
    RenderEngine render_engine;


    public JetCad() {

        super("JetCad");
        GlobalData.configData.CurrentWorkbench = "JetCad";
        setSize(1100, 800);
        setExtendedState(JFrame.MAXIMIZED_BOTH);
        setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
        setLocationRelativeTo(null);

        ui_widgets = new UIWidgets();
        render_engine = new RenderEngine();
        
        Layout_UI();
        createMenuBar();
        setJMenuBar(menu_bar);
        JetCadViewerPanel panel = new JetCadViewerPanel();
        add(panel);
        panel.addMouseListener(new MouseAdapter() {
            @Override
            public void mousePressed(MouseEvent e) {
                //System.out.println(e.getX() + "," + e.getY());
                ui_widgets.ClickPressStack(e.getX(), e.getY());
                GlobalData.MousePositionX_MCS = (GlobalData.MousePositionX - GlobalData.ViewerPan[0]) / GlobalData.ViewerZoom;
                GlobalData.MousePositionY_MCS = ((GlobalData.MousePositionY - GlobalData.ViewerPan[1]) / GlobalData.ViewerZoom) * -1;
                render_engine.ClickPressStack(GlobalData.MousePositionX_MCS, GlobalData.MousePositionY_MCS);
                repaint();
            }
            public void mouseReleased(MouseEvent e) {
                //System.out.println(e.getX() + "," + e.getY());
                ui_widgets.ClickReleaseStack(e.getX(), e.getY());
                GlobalData.MousePositionX_MCS = (GlobalData.MousePositionX - GlobalData.ViewerPan[0]) / GlobalData.ViewerZoom;
                GlobalData.MousePositionY_MCS = ((GlobalData.MousePositionY - GlobalData.ViewerPan[1]) / GlobalData.ViewerZoom) * -1;
                render_engine.ClickReleaseStack(GlobalData.MousePositionX_MCS, GlobalData.MousePositionY_MCS);
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
                render_engine.MouseMotionStack(GlobalData.MousePositionX_MCS, GlobalData.MousePositionY_MCS);
                repaint();
            }
            public void mouseDragged(MouseEvent e) {
                //System.out.println(e.getX() + "," + e.getY());
                GlobalData.MousePositionX = e.getX();
                GlobalData.MousePositionY = e.getY();
                ui_widgets.MouseMotionStack(e.getX(), e.getY());
                GlobalData.MousePositionX_MCS = (GlobalData.MousePositionX - GlobalData.ViewerPan[0]) / GlobalData.ViewerZoom;
                GlobalData.MousePositionY_MCS = ((GlobalData.MousePositionY - GlobalData.ViewerPan[1]) / GlobalData.ViewerZoom) * -1;
                render_engine.MouseMotionStack(GlobalData.MousePositionX_MCS, GlobalData.MousePositionY_MCS);
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
                        if (!GlobalData.configData.CurrentWorkbench.contentEquals("JetCad")) return false;
                        switch (ke.getID()) {
                            case KeyEvent.KEY_PRESSED:

                                //repaint();
                                break;

                            case KeyEvent.KEY_RELEASED:

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

        menuItem = new JMenuItem("New Drawing");
        menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_0, ActionEvent.ALT_MASK));
        menuItem.getAccessibleContext().setAccessibleDescription("New Drawing File");
        menuItem.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {

                repaint();
            }
        });
        menu.add(menuItem);

        menuItem = new JMenuItem("Open Drawing");
        menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_1, ActionEvent.ALT_MASK));
        menuItem.getAccessibleContext().setAccessibleDescription("Open Drawing File");
        menuItem.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {

                repaint();
            }
        });
        menu.add(menuItem);

        menuItem = new JMenuItem("Save Drawing");
        menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_2, ActionEvent.ALT_MASK));
        menuItem.getAccessibleContext().setAccessibleDescription("Save Drawing File");
        menuItem.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (GlobalData.configData.JetToolpathJobFile == null) {
                    JFileChooser fileChooser = new JFileChooser();
                    fileChooser.setCurrentDirectory(new File("."));
                    fileChooser.addChoosableFileFilter(new FileFilter() {

                        public String getDescription() {
                            return "JetCad Drawing File (*.jcad)";
                        }

                        public boolean accept(File f) {
                            if (f.isDirectory()) {
                                return true;
                            } else {
                                return f.getName().toLowerCase().endsWith(".jcad");
                            }
                        }
                    });
                    if (fileChooser.showSaveDialog(getParent()) == JFileChooser.APPROVE_OPTION) {
                        File file = fileChooser.getSelectedFile();
                        GlobalData.configData.JetToolpathJobFile = file.getAbsolutePath();
                        //toolpath_viewer.SaveJob(GlobalData.configData.JetToolpathJobFile);
                        try {
                            GlobalData.pushConfig();
                        } catch (FileNotFoundException e1) {
                            e1.printStackTrace();
                        }
                    }
                }
                else
                {
                    //toolpath_viewer.SaveJob(GlobalData.configData.JetToolpathJobFile);
                }
            }
        });
        menu.add(menuItem);

        menuItem = new JMenuItem("Save Drawing As");
        menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_3, ActionEvent.ALT_MASK));
        menuItem.getAccessibleContext().setAccessibleDescription("Save Drawing File As");
        menuItem.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                JFileChooser fileChooser = new JFileChooser();
                fileChooser.setCurrentDirectory(new File("."));
                fileChooser.addChoosableFileFilter(new FileFilter() {

                    public String getDescription() {
                        return "Xmotion Job File (*.jcad)";
                    }

                    public boolean accept(File f) {
                        if (f.isDirectory()) {
                            return true;
                        } else {
                            return f.getName().toLowerCase().endsWith(".jcad");
                        }
                    }
                });
                if (fileChooser.showSaveDialog(getParent()) == JFileChooser.APPROVE_OPTION) {
                    File file = fileChooser.getSelectedFile();
                    GlobalData.configData.JetToolpathJobFile = file.getAbsolutePath();
                    //toolpath_viewer.SaveJob(GlobalData.configData.JetToolpathJobFile);
                }
            }
        });
        menu.add(menuItem);

        menuItem = new JMenuItem("Import DXF");
        menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_4, ActionEvent.ALT_MASK));
        menuItem.getAccessibleContext().setAccessibleDescription("Import DXF Drawing");
        menuItem.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                JFileChooser fileChooser = new JFileChooser();
                fileChooser.setCurrentDirectory(new File("."));
                int result = fileChooser.showOpenDialog(getParent());
                if (result == JFileChooser.APPROVE_OPTION) {
                    File selectedFile = fileChooser.getSelectedFile();
                    System.out.println("Selected file: " + selectedFile.getAbsolutePath());

                }
            }
        });
        menu.add(menuItem);

        menuItem = new JMenuItem("Import SVG");
        menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_4, ActionEvent.ALT_MASK));
        menuItem.getAccessibleContext().setAccessibleDescription("Import SVG Drawing");
        menuItem.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                JFileChooser fileChooser = new JFileChooser();
                fileChooser.setCurrentDirectory(new File("."));
                int result = fileChooser.showOpenDialog(getParent());
                if (result == JFileChooser.APPROVE_OPTION) {
                    File selectedFile = fileChooser.getSelectedFile();
                    System.out.println("Selected file: " + selectedFile.getAbsolutePath());

                }
            }
        });
        menu.add(menuItem);
    }
    private void Layout_UI()
    {

    }
    // create a panel that you can draw on.
    class JetCadViewerPanel extends JPanel {
        public void paint(Graphics g) {
            Rectangle Frame_Bounds = this.getParent().getBounds();
            GlobalData.MousePositionX_MCS = (GlobalData.MousePositionX - GlobalData.ViewerPan[0]) / GlobalData.ViewerZoom;
            GlobalData.MousePositionY_MCS = ((GlobalData.MousePositionY - GlobalData.ViewerPan[1]) / GlobalData.ViewerZoom) * -1;
            Graphics2D g2d = (Graphics2D) g;
            /* Begin Wallpaper */
            g.setColor(Color.DARK_GRAY);
            g.fillRect(0,0,Frame_Bounds.width,Frame_Bounds.height);
            /* End Wallpaper */
            render_engine.RenderStack(g2d);
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
                new JetCad().setVisible(true);
            }
        });
    }
}