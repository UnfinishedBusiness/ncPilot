package com.nc.pilot.ui;

import com.nc.pilot.dialogs.JetToolpathCutChart;
import com.nc.pilot.dialogs.JetToolpathJobSetup;
import com.nc.pilot.lib.GlobalData;
import com.nc.pilot.lib.ToolPathViewer.ToolpathViewer;
import com.nc.pilot.lib.UIWidgets.UIWidgets;
import org.kabeja.parser.ParseException;

import javax.swing.*;
import javax.swing.Timer;
import javax.swing.filechooser.FileFilter;
import java.awt.*;
import java.awt.event.*;
import java.io.File;
import java.io.FileNotFoundException;
import java.util.*;

public class MotionSimulator extends JFrame {
    JMenuBar menu_bar;
    UIWidgets ui_widgets;

    java.util.Timer interupt_timer = new java.util.Timer();

    int[] target_position;
    int[] machine_position;
    int dx, dy, err, e2, sx, sy, x0, x1, y0, y1;

    void set_target_position(int x, int y)
    {
        target_position = new int[]{x, y};
        x1 = target_position[0];
        y1 = target_position[1];
        x0 = machine_position[0];
        y0 = machine_position[1];
        dx = Math.abs(x1-x0);
        sx = x0<x1 ? 1 : -1;
        dy = Math.abs(y1-y0);
        sy = y0<y1 ? 1 : -1;
        err = (dx>dy ? dx : -dy)/2;
    }

    private void interupt()
    {
        //setPixel(x0,y0);
        machine_position[0] = x0;
        machine_position[1] = y0;
        if (x0==x1 && y0==y1)
        {

        }
        else
        {
            e2 = err;
            if (e2 >-dx) { err -= dy; x0 += sx; }
            if (e2 < dy) { err += dx; y0 += sy; }
        }
        repaint();
    }

    public MotionSimulator() {

        super("Motion Simulator");
        //GlobalData.configData.CurrentWorkbench = "MotionSimulator";
        setSize(1100, 800);
        setExtendedState(JFrame.MAXIMIZED_BOTH);
        setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
        setLocationRelativeTo(null);

        ui_widgets = new UIWidgets();
        
        Layout_UI();
        createMenuBar();
        setJMenuBar(menu_bar);
        ToolpathViewerPanel panel = new ToolpathViewerPanel();
        add(panel);

        machine_position = new int[] {100, 100};

        set_target_position(300, 200);
        interupt_timer.schedule(new TimerTask() {
            @Override
            public void run() {
                interupt();
            }
        }, 0, 25);


        KeyboardFocusManager.getCurrentKeyboardFocusManager()
                .addKeyEventDispatcher(new KeyEventDispatcher() {
                    @Override
                    public boolean dispatchKeyEvent(KeyEvent ke) {
                        //if (!GlobalData.configData.CurrentWorkbench.contentEquals("MotionSimulator")) return false;
                        switch (ke.getID()) {
                            case KeyEvent.KEY_PRESSED:
                                //System.out.println("(Jet Toolpath) Key: " + ke.getKeyCode());
                                if (ke.getKeyCode() == 44) //< key
                                {

                                    repaint();
                                }
                                if (ke.getKeyCode() == 46) //> key
                                {

                                    repaint();
                                }

                                //repaint();
                                break;

                            case KeyEvent.KEY_RELEASED:
                                if (ke.getKeyCode() == KeyEvent.VK_SPACE) {
                                    set_target_position(200, 400);
                                    repaint();
                                }
                                break;
                        }
                        return false;
                    }
                });

        // call onCancel() when cross is clicked
        setDefaultCloseOperation(EXIT_ON_CLOSE);
        addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent e) {
                onClose();
            }
        });

    }
    private void onClose() {
        // add your code here if necessary
        //GlobalData.configData.CurrentWorkbench = "MachineControl";
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
                    //selectedFile.getAbsolutePath();
                    repaint();
                }
            }
        });
        menu.add(menuItem);
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
            g.setFont(new Font("Arial", Font.PLAIN, 40));
            g.setColor(Color.green);
            g.drawString("+", machine_position[0], machine_position[1]);
            ui_widgets.RenderStack(g2d, Frame_Bounds);
            //Display Mouse position in MCS and Screen Cord

            g.setColor(Color.green);
            g.setFont(new Font("Arial", Font.BOLD, 12));
            g.drawString(String.format("DRO MCS-> X: %d Y: %d", machine_position[0], machine_position[1]), 10, 10);
            g.drawString(String.format("DRO Move DTG-> X: %d Y: %d", dx, dy), 10, 25);
        }
    }
    public static void main(String[] args) {
        SwingUtilities.invokeLater(new Runnable() {
            @Override
            public void run() {
                new MotionSimulator().setVisible(true);
            }
        });
    }
}