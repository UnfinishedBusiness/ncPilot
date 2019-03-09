package com.nc.pilot.lib.MDIConsole;
import com.nc.pilot.lib.GcodeViewer;
import com.nc.pilot.lib.GlobalData;
import com.nc.pilot.lib.MotionController.MotionController;

import java.awt.*;
import java.awt.event.KeyEvent;
import java.util.ArrayList;

/**
 * Created by admin on 2/10/19.
 */

public class MDIConsole {
    private Graphics2D g;
    private Rectangle Frame_Bounds;
    public boolean isVisible = false;
    private String cmd_line = "";
    private ArrayList<MDICommand> CommandStack = new ArrayList();
    private ArrayList<String> RecievedLines = new ArrayList();


    public MDIConsole()
    {
        AddCommand("close", new Runnable() {
            @Override
            public void run() {
                System.out.println("Arguments: " + get_args("close"));
                hide();
            }
        });
        AddCommand("probez", new Runnable() {
            @Override
            public void run() {
                MotionController.BlockNextStatusReports = 1;
                MotionController.WriteBufferAndRunAfterStop("G92.2\nG38.2 Z-10 F40\n", new Runnable() {
                    @Override
                    public void run() {
                        System.out.println("Z Probed!");
                        MotionController.BlockNextStatusReports = 1;
                        MotionController.WriteBuffer("G92.3\nG92 Z=0\n");
                        MotionController.WriteBuffer("G90\nG0 Z0.375\n");
                    }
                });

            }
        });
    }
    // Uses font metrics provided by the current font set to the
    // local Graphics2D to find the width of a string in pixels.
    private int calculateTextWidth(String text) {
        return g.getFontMetrics().stringWidth(text);
    }

    public void RecieveBufferLine(String line)
    {
        //System.out.println("Read Buffer: " + line);
        if (RecievedLines.size() < 30)
        {
            RecievedLines.add(line);
        }
        else
        {
            ArrayList<String> tmp = new ArrayList();
            for (int x = 1; x < RecievedLines.size(); x++)
            {
                tmp.add(RecievedLines.get(x));
            }
            tmp.add(line);
            RecievedLines.clear();
            for (int x = 0; x < tmp.size(); x++)
            {
                RecievedLines.add(tmp.get(x));
            }
        }
    }
    private void AddCommand(String cmd, Runnable action)
    {
        MDICommand c = new MDICommand();
        c.name = cmd;
        c.action = action;
        CommandStack.add(c);
    }
    private void DrawConsole()
    {
        Color c=new Color(0.1f, 0.1f, 0.1f, 1f);
        g.setColor(c);
        g.fillRect(50, 50, Frame_Bounds.width - 200, Frame_Bounds.height - 100);
        g.setFont(new Font("Arial", Font.BOLD, 30));
        g.setColor(Color.white);
        g.drawString(">", 70, 90);
        g.drawString(cmd_line, 120, 90);

        g.setFont(new Font("Arial", Font.BOLD, 15));
        int y_offset = 140;
        for (int x = 0; x < RecievedLines.size(); x++)
        {
            g.drawString(RecievedLines.get(x), 160, y_offset);
            y_offset += 15;
        }
    }
    public void RenderStack(Graphics2D graphics, Rectangle f){
        g = graphics;
        Frame_Bounds = f;
        if (isVisible == true) DrawConsole();
    }
    public void show()
    {
        isVisible = true;
    }
    public void hide()
    {
        isVisible = false;
    }
    private String get_args(String cmd_line)
    {
        String[] cmd = cmd_line.split("\\s+");
        if (cmd[0] != null)
        {
            for (int x = 0; x < CommandStack.size(); x++)
            {
                if (CommandStack.get(x).name.contentEquals(cmd[0]))
                {
                    return CommandStack.get(x).args;
                }
            }
        }
        return "";
    }
    private void evaluate_command()
    {
        String[] cmd = cmd_line.split("\\s+");
        boolean found_in_stack = false;
        if (cmd[0] != null)
        {
            for (int x = 0; x < CommandStack.size(); x++)
            {
                if (CommandStack.get(x).name.contentEquals(cmd[0]))
                {
                    cmd[0] = "";
                    if (cmd.length > 1)
                    {
                        CommandStack.get(x).args = String.join(" ", cmd).substring(1);
                    }
                    else
                    {
                        CommandStack.get(x).args = "";
                    }
                    CommandStack.get(x).action.run();
                    found_in_stack = true;
                }
            }
        }
        if (found_in_stack == false) //We are a MDI command, send to Motion Controller
        {
            MotionController.WriteBuffer(cmd_line + "\n");
        }
    }
    public void dispatchKeyEvent(KeyEvent ke)
    {
        if (isVisible == true)
        {
            switch (ke.getID()) {
                case KeyEvent.KEY_PRESSED:
                    if (ke.getKeyCode() == KeyEvent.VK_BACK_SPACE)
                    {
                        if (cmd_line.length() > 0)
                        {
                            cmd_line = cmd_line.substring(0, cmd_line.length() - 1);
                        }
                    }
                    else if (ke.getKeyCode() == KeyEvent.VK_SPACE)
                    {
                        cmd_line = cmd_line + " ";
                    }
                    else if (ke.getKeyCode() == KeyEvent.VK_ENTER)
                    {
                        evaluate_command();
                        cmd_line = "";
                    }
                    else
                    {
                        char c = ke.getKeyChar();
                        if (Character.isAlphabetic(c) || Character.isDigit(c) || c == '.' || c == '=' || c == '-' || c == '$' || c == '?' || c == '~' || c == '!' || c == '#' || c == '*')
                        {
                            cmd_line = cmd_line + c;
                        }
                    }
                    break;

                case KeyEvent.KEY_RELEASED:
                    if (ke.getKeyCode() == KeyEvent.VK_ESCAPE) {
                        hide();
                    }
                    break;
            }
        }
    }
}