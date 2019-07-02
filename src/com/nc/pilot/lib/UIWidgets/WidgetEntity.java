package com.nc.pilot.lib.UIWidgets;

/**
 * Created by travis on 2/10/19.
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
    public String unit_name;
    public String value;

    public Runnable action;

    //Meta data
    public int real_posx;
    public int real_posy;
    public boolean engaged;
    public boolean visable;
}