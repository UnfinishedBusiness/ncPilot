package com.nc.pilot.lib;

import java.awt.*;
import java.util.ArrayList;
import java.util.concurrent.Callable;

/**
 * Created by travis on 2/4/2019.
 */
class WidgetEntity {
    public String type;
    public String group;
    public String anchor; //Right or Left
    public String text;

    public int width;
    public int height;
    public int posx;
    public int posy;
    public int min;
    public int max;
    public int position;

    public Runnable action;

    //Meta data
    public int real_posx;
    public int real_posy;
    public boolean engaged;
}

public class UIWidgets {
    private ArrayList<WidgetEntity> WidgetStack = new ArrayList();
    private Graphics2D g;
    private Rectangle Frame_Bounds;
    private static boolean isMousePressed = false;
    private int[] mouseLastDragPosition;
    public UIWidgets()
    {

    }

    int map(int x, int in_min, int in_max, int out_min, int out_max)
    {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    // Uses font metrics provided by the current font set to the
    // local Graphics2D to find the width of a string in pixels.
    private int calculateTextWidth(String text) {
        return g.getFontMetrics().stringWidth(text);
    }

    public void DrawDRO()
    {
        g.setFont(new Font("Arial", Font.BOLD, 45));
        if (GlobalData.IsHomed == false)
        {
            g.setColor(Color.red);
        }
        else
        {
            g.setColor(Color.green);
        }
        int DRO_X_Offset = -30;
        g.drawString("X:", Frame_Bounds.width - 350 - DRO_X_Offset, 70);
        g.drawString("Y:", Frame_Bounds.width - 350 - DRO_X_Offset, 140);
        g.drawString("Z:", Frame_Bounds.width - 350 - DRO_X_Offset, 210);

        g.drawString(String.format("%.4f", GlobalData.dro[0]), Frame_Bounds.width - 220 - DRO_X_Offset, 70);
        g.drawString(String.format("%.4f", GlobalData.dro[1]), Frame_Bounds.width - 220 - DRO_X_Offset, 140);
        g.drawString(String.format("%.4f", GlobalData.dro[2]), Frame_Bounds.width - 220 - DRO_X_Offset, 210);
        g.drawRect(Frame_Bounds.width - 360, 10, 350, 240);
    }
    public void DrawButton(String text, boolean engaged, int width, int height, int posx, int posy) {
        //System.out.println("Drawing " + text);
        int button_font_size = 15;
        if (engaged == true)
        {
            g.setColor(Color.green);
        }
        else
        {
            g.setColor(Color.red);
        }
        g.setFont(new Font("Arial", Font.PLAIN, button_font_size));
        int text_length = (text.length() - 3) * button_font_size;
        int text_posx = (width / 2) - (calculateTextWidth(text) / 2) + posx;
        int text_posy = posy + (height / 2) + (button_font_size/2);
        g.drawString(text, text_posx, text_posy);
        g.drawRect(posx, posy, width, height);
    }

    public void DrawSlider(String text, boolean engaged, int width, int height, int real_posx, int real_posy, int position, int min, int max){
        g.setColor(Color.red);
        int button_font_size = 15;
        g.setFont(new Font("Arial", Font.PLAIN, button_font_size));
        g.drawString(text, real_posx + 20, real_posy + 20);
        g.drawRect(real_posx, real_posy, width, height); //Border

        int slider_leftmost = real_posx + 20;
        int slider_rail_width = width - 40;
        g.drawRect(slider_leftmost, real_posy + 40, slider_rail_width, 5); //Slider Rail

        //int slider_offset = map(position, min, max, 0, slider_rail_width);
        int slider_offset = position;
        if (slider_offset > slider_rail_width - 15) slider_offset = slider_rail_width - 15;
        if (slider_offset < 0) slider_offset = 0;
        //System.out.println("slider_offset: " + slider_offset);
        //if (slider_offset < slider_leftmost) slider_offset = slider_leftmost;
        //System.out.println(position);
        //int slider_offset = 50;
        if (engaged == true)
        {
            g.setColor(Color.green);
        }
        else
        {
            g.setColor(Color.red);
        }
        g.drawRect( slider_leftmost + slider_offset, real_posy + 35, 15, 15); //Slider
    }
    
    public void AddMomentaryButton(String text, String anchor, int width, int height, int posx, int posy, Runnable action){
        //System.out.println("Adding: " + text);
        WidgetEntity w = new WidgetEntity();
        w.type = "momentary_button";
        w.anchor = anchor;
        w.text = text;
        w.engaged = false;
        w.width = width;
        w.height = height;
        w.posx = posx;
        w.posy = posy;
        w.action = action;
        WidgetStack.add(w);
    }
    public void AddSelectButton(String text, String group, boolean isDefault, String anchor, int width, int height, int posx, int posy, Runnable action){
        //System.out.println("Adding: " + text);
        WidgetEntity w = new WidgetEntity();
        w.type = "select_button";
        w.anchor = anchor;
        w.text = text;
        w.engaged = false;
        w.width = width;
        w.height = height;
        w.posx = posx;
        w.posy = posy;
        w.action = action;
        w.group = group;
        if (isDefault) w.engaged = true;
        WidgetStack.add(w);
    }
    public void AddSlider(String text, String anchor, int width, int height, int posx, int posy, int min, int max, int defaultPosition, Runnable action){
        //System.out.println("Adding: " + text);
        WidgetEntity w = new WidgetEntity();
        w.type = "slider";
        w.anchor = anchor;
        w.text = text;
        w.engaged = false;
        w.width = width;
        w.height = height;
        w.posx = posx;
        w.posy = posy;
        w.action = action;
        w.min = min;
        w.max = max;
        w.position = map(defaultPosition, min, max, 0, width);
        w.engaged = false;
        WidgetStack.add(w);
    }
    public int getSliderPosition(String text)
    {
        for (int x = 0; x < WidgetStack.size(); x++)
        {
            if (WidgetStack.get(x).text.equals(text)){
                //System.out.println("position in normal units: " + WidgetStack.get(x).position);
                //System.out.println("width: " + WidgetStack.get(x).width);
                //System.out.println("min: " + WidgetStack.get(x).min);
                //System.out.println("max: " + WidgetStack.get(x).max);
                return map(WidgetStack.get(x).position, 0, WidgetStack.get(x).width, WidgetStack.get(x).min, WidgetStack.get(x).max);
            }
        }
        return -1;
    }
    public void RenderStack(Graphics2D graphics, Rectangle f){
        g = graphics;
        Frame_Bounds = f;
        //System.out.println("WidgetStack has " + WidgetStack.size() + " Entities!");
        for (int x = 0; x < WidgetStack.size(); x++)
        {
            //System.out.println("Rendering " + x + " Entity which is a " + WidgetStack.get(x).type + " with a text of " + WidgetStack.get(x).text);
            if (WidgetStack.get(x).type.equals("momentary_button")){
                if (WidgetStack.get(x).anchor.equals("top-right")){
                    WidgetStack.get(x).real_posx = Frame_Bounds.width - WidgetStack.get(x).posx - WidgetStack.get(x).width;
                    WidgetStack.get(x).real_posy = WidgetStack.get(x).posy;
                }
                else if (WidgetStack.get(x).anchor.equals("bottom-right")){
                    WidgetStack.get(x).real_posx = Frame_Bounds.width - WidgetStack.get(x).posx - WidgetStack.get(x).width;
                    WidgetStack.get(x).real_posy = Frame_Bounds.height - WidgetStack.get(x).posy - WidgetStack.get(x).height;
                }
                else {
                    WidgetStack.get(x).real_posx = WidgetStack.get(x).posx;
                    WidgetStack.get(x).real_posy = WidgetStack.get(x).posy;
                }
                DrawButton(WidgetStack.get(x).text, WidgetStack.get(x).engaged, WidgetStack.get(x).width, WidgetStack.get(x).height, WidgetStack.get(x).real_posx, WidgetStack.get(x).real_posy);
            }
            if (WidgetStack.get(x).type.equals("select_button")){
                if (WidgetStack.get(x).anchor.equals("top-right")){
                    WidgetStack.get(x).real_posx = Frame_Bounds.width - WidgetStack.get(x).posx - WidgetStack.get(x).width;
                    WidgetStack.get(x).real_posy = WidgetStack.get(x).posy;
                }
                else if (WidgetStack.get(x).anchor.equals("bottom-right")){
                    WidgetStack.get(x).real_posx = Frame_Bounds.width - WidgetStack.get(x).posx - WidgetStack.get(x).width;
                    WidgetStack.get(x).real_posy = Frame_Bounds.height - WidgetStack.get(x).posy - WidgetStack.get(x).height;
                }
                else {
                    WidgetStack.get(x).real_posx = WidgetStack.get(x).posx;
                    WidgetStack.get(x).real_posy = WidgetStack.get(x).posy;
                }
                DrawButton(WidgetStack.get(x).text, WidgetStack.get(x).engaged, WidgetStack.get(x).width, WidgetStack.get(x).height, WidgetStack.get(x).real_posx, WidgetStack.get(x).real_posy);
            }
            if (WidgetStack.get(x).type.equals("slider")){
                if (WidgetStack.get(x).anchor.equals("top-right")){
                    WidgetStack.get(x).real_posx = Frame_Bounds.width - WidgetStack.get(x).posx - WidgetStack.get(x).width;
                    WidgetStack.get(x).real_posy = WidgetStack.get(x).posy;
                }
                else if (WidgetStack.get(x).anchor.equals("bottom-right")){
                    WidgetStack.get(x).real_posx = Frame_Bounds.width - WidgetStack.get(x).posx - WidgetStack.get(x).width;
                    WidgetStack.get(x).real_posy = Frame_Bounds.height - WidgetStack.get(x).posy - WidgetStack.get(x).height;
                }
                else {
                    WidgetStack.get(x).real_posx = WidgetStack.get(x).posx;
                    WidgetStack.get(x).real_posy = WidgetStack.get(x).posy;
                }
                DrawSlider(WidgetStack.get(x).text, WidgetStack.get(x).engaged, WidgetStack.get(x).width, WidgetStack.get(x).height, WidgetStack.get(x).real_posx, WidgetStack.get(x).real_posy, WidgetStack.get(x).position, WidgetStack.get(x).min, WidgetStack.get(x).max);
            }
        }
        DrawDRO();

    }
    public void ClickPressStack(int mousex, int mousey){
        isMousePressed = true;
        for (int x = 0; x < WidgetStack.size(); x++)
        {
            if (WidgetStack.get(x).type.equals("momentary_button")){
                if ((mousex > WidgetStack.get(x).real_posx && mousex < WidgetStack.get(x).real_posx + WidgetStack.get(x).width) && (mousey > WidgetStack.get(x).real_posy && mousey < WidgetStack.get(x).real_posy + WidgetStack.get(x).height))
                {
                    //System.out.println("Clicked on: " + WidgetStack.get(x).text);
                    WidgetStack.get(x).engaged = true;
                }
            }
            if (WidgetStack.get(x).type.equals("slider")){
                if ((mousex > WidgetStack.get(x).real_posx && mousex < WidgetStack.get(x).real_posx + WidgetStack.get(x).width) && (mousey > WidgetStack.get(x).real_posy && mousey < WidgetStack.get(x).real_posy + WidgetStack.get(x).height))
                {
                    //System.out.println("Clicked on: " + WidgetStack.get(x).text);
                    WidgetStack.get(x).engaged = true;
                }
            }
            if (WidgetStack.get(x).type.equals("select_button")){
                if ((mousex > WidgetStack.get(x).real_posx && mousex < WidgetStack.get(x).real_posx + WidgetStack.get(x).width) && (mousey > WidgetStack.get(x).real_posy && mousey < WidgetStack.get(x).real_posy + WidgetStack.get(x).height))
                {
                    //System.out.println("Clicked on: " + WidgetStack.get(x).text);
                    WidgetStack.get(x).engaged = true;
                    String group = WidgetStack.get(x).group;
                    for (int i = 0; i < WidgetStack.size(); i++)
                    {
                        if (WidgetStack.get(i).type.equals("select_button") && WidgetStack.get(i).group.equals(group) && WidgetStack.get(i).text != WidgetStack.get(x).text){
                            WidgetStack.get(i).engaged = false;
                        }
                    }
                }
            }
        }
    }
    public void ClickReleaseStack(int mousex, int mousey){
        isMousePressed = false;
        for (int x = 0; x < WidgetStack.size(); x++)
        {
            if (WidgetStack.get(x).type.equals("momentary_button")){
                if ((mousex > WidgetStack.get(x).real_posx && mousex < WidgetStack.get(x).real_posx + WidgetStack.get(x).width) && (mousey > WidgetStack.get(x).real_posy && mousey < WidgetStack.get(x).real_posy + WidgetStack.get(x).height))
                {
                    WidgetStack.get(x).engaged = false;
                    WidgetStack.get(x).action.run();
                }
            }
            if (WidgetStack.get(x).type.equals("select_button")){
                if ((mousex > WidgetStack.get(x).real_posx && mousex < WidgetStack.get(x).real_posx + WidgetStack.get(x).width) && (mousey > WidgetStack.get(x).real_posy && mousey < WidgetStack.get(x).real_posy + WidgetStack.get(x).height))
                {
                    WidgetStack.get(x).action.run();
                }
            }
            if (WidgetStack.get(x).type.equals("slider")){
                if ((mousex > WidgetStack.get(x).real_posx && mousex < WidgetStack.get(x).real_posx + WidgetStack.get(x).width) && (mousey > WidgetStack.get(x).real_posy && mousey < WidgetStack.get(x).real_posy + WidgetStack.get(x).height))
                {
                    WidgetStack.get(x).engaged = false;
                    WidgetStack.get(x).action.run();
                }
            }
        }
    }
    public void MouseMotionStack(int mousex, int mousey){
        if (isMousePressed == true)
        {
            for (int x = 0; x < WidgetStack.size(); x++)
            {
                if (WidgetStack.get(x).type.equals("slider")){
                    if ((mousex > WidgetStack.get(x).real_posx && mousex < WidgetStack.get(x).real_posx + WidgetStack.get(x).width) && (mousey > WidgetStack.get(x).real_posy && mousey < WidgetStack.get(x).real_posy + WidgetStack.get(x).height))
                    {
                        int x_drag = mousex - mouseLastDragPosition[0];
                        if (Math.abs(x_drag) < 60)
                        {
                            WidgetStack.get(x).position += x_drag;
                            if (WidgetStack.get(x).position > WidgetStack.get(x).width)
                            {
                                WidgetStack.get(x).position = WidgetStack.get(x).width;
                            }
                            if (WidgetStack.get(x).position < 0)
                            {
                                WidgetStack.get(x).position = 0;
                            }
                        }
                        //System.out.println("Mouse drag: " + x_drag);
                    }
                }
            }
        }
        mouseLastDragPosition = new int[]{mousex, mousey};
    }
}
