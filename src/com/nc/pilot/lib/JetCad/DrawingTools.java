package com.nc.pilot.lib.JetCad;

import com.nc.pilot.lib.JetCad.DrawingStack.DrawingEntity;
import com.nc.pilot.lib.JetCad.DrawingStack.RenderEngine;
import com.nc.pilot.lib.UIWidgets.UIWidgets;

import java.util.ArrayList;

public class DrawingTools {
    private ArrayList<DrawingToolStruct> ToolStack = new ArrayList();
    private RenderEngine render_engine;
    private UIWidgets ui_widgets;

    private void AddTool(String name, String hotkey, Runnable action)
    {
        DrawingToolStruct tool = new DrawingToolStruct();
        tool.name = name;
        tool.hotkey = hotkey;
        tool.action = action;
        ToolStack.add(tool);
    }
    private void SelectAll()
    {
        for (int x = 0; x < render_engine.DrawingStack.size(); x++)
        {
            render_engine.DrawingStack.get(x).isSelected = true;
        }
    }
    private void UnSelectAll()
    {
        for (int x = 0; x < render_engine.DrawingStack.size(); x++)
        {
            render_engine.DrawingStack.get(x).isSelected = false;
        }
    }
    private void SaveDrawing()
    {

    }
    private void CopySelected()
    {

    }
    private void PasteSelected()
    {

    }
    private void Undo()
    {

    }
    private void DeleteSelected()
    {
        System.out.println("Deleting selected!");
        ArrayList<DrawingEntity> temp_stack = new ArrayList();
        for (int x = 0; x < render_engine.DrawingStack.size(); x++)
        {
            if (render_engine.DrawingStack.get(x).isSelected == false)
            {
                temp_stack.add(render_engine.DrawingStack.get(x));
            }
        }
        render_engine.DrawingStack.clear();
        for (int x = 0; x < temp_stack.size(); x++)
        {
            render_engine.DrawingStack.add(temp_stack.get(x));
        }
    }
    private void DrawLine()
    {
        System.out.println("Drawing line!");
    }
    private void DrawRectangle()
    {

    }
    private void DrawCircle()
    {

    }
    private void Trim()
    {

    }
    private void Fillet()
    {

    }
    private void Offset()
    {

    }
    private void Move()
    {

    }
    private void ToggleConstructionStyle()
    {

    }
    private void SearchToolBox()
    {
        ui_widgets.setInputBoxVisability("Tool", true);
        ui_widgets.setInputBoxEngaged("Tool", true);
    }
    private void ParallelLine()
    {

    }
    private void Escape()
    {
        ui_widgets.setInputBoxVisability("Tool", false);
        ui_widgets.setInputBoxEngaged("Tool", false);
        ui_widgets.setInputBoxValue("Tool", "");
        UnSelectAll();
    }
    private void ChainSelect()
    {

    }
    private void Rotate()
    {

    }
    private void CircularPattern()
    {

    }
    private void Scale()
    {

    }
    private void Mirror()
    {

    }
    private void Color()
    {

    }
    private void Layer()
    {

    }
    private void Text()
    {

    }
    private void DrawTextAlongPath()
    {

    }
    private void CleanDrawing()
    {

    }
    private void Slot()
    {

    }
    private void Chamfer()
    {

    }
    private void Fit()
    {

    }
    private void InvertDelete()
    {

    }
    private void RemoveDuplicates()
    {

    }
    private void LatheMode()
    {

    }
    private void Tab()
    {

    }
    public DrawingTools(RenderEngine r, UIWidgets u)
    {
        render_engine = r;
        ui_widgets = u;
        //Add a few enties to test rendering engine
        DrawingEntity e;

        e = new DrawingEntity();
        e.type = "line";
        e.start = new float[]{5, 5};
        e.end = new float[]{25, 15};
        render_engine.DrawingStack.add(e);

        e = new DrawingEntity();
        e.type = "line";
        e.start = new float[]{25, 15};
        e.end = new float[]{20, 5};
        render_engine.DrawingStack.add(e);

        e = new DrawingEntity();
        e.type = "line";
        e.start = new float[]{20, 5};
        e.end = new float[]{5, 5};
        render_engine.DrawingStack.add(e);

        AddTool("select_all", "ctrl-a", new Runnable() {
            @Override
            public void run() {
                SelectAll();
            }
        });
        AddTool("save_drawing", "ctrl-s", new Runnable() {
            @Override
            public void run() {
                SaveDrawing();
            }
        });
        AddTool("copy", "ctrl-c", new Runnable() {
            @Override
            public void run() {
                CopySelected();
            }
        });
        AddTool("paste", "ctrl-v", new Runnable() {
            @Override
            public void run() {
                PasteSelected();
            }
        });
        AddTool("undo", "ctrl-z", new Runnable() {
            @Override
            public void run() {
                Undo();
            }
        });
        AddTool("delete", "ctrl-d", new Runnable() {
            @Override
            public void run() {
                DeleteSelected();
            }
        });
        AddTool("line", "l", new Runnable() {
            @Override
            public void run() {
                DrawLine();
            }
        });
        AddTool("rectangle", "r", new Runnable() {
            @Override
            public void run() {
                DrawRectangle();
            }
        });
        AddTool("circle", "c", new Runnable() {
            @Override
            public void run() {
                DrawCircle();
            }
        });
        AddTool("trim", "t", new Runnable() {
            @Override
            public void run() {
                Trim();
            }
        });
        AddTool("fillet", "f", new Runnable() {
            @Override
            public void run() {
                Fillet();
            }
        });
        AddTool("offset", "o", new Runnable() {
            @Override
            public void run() {
                Offset();
            }
        });
        AddTool("move", "m", new Runnable() {
            @Override
            public void run() {
                Move();
            }
        });
        AddTool("", "x", new Runnable() {
            @Override
            public void run() {
                ToggleConstructionStyle();
            }
        });
        AddTool("", "s", new Runnable() {
            @Override
            public void run() {
                SearchToolBox();
            }
        });
        AddTool("parallel_line", "p", new Runnable() {
            @Override
            public void run() {
                ParallelLine();
            }
        });
        AddTool("", "Escape", new Runnable() {
            @Override
            public void run() {
                Escape();
            }
        });
        AddTool("chain_select", "Space", new Runnable() {
            @Override
            public void run() {
                ChainSelect();
            }
        });
        AddTool("rotate", "", new Runnable() {
            @Override
            public void run() {
                Rotate();
            }
        });
        AddTool("circular_pattern", "", new Runnable() {
            @Override
            public void run() {
                CircularPattern();
            }
        });
        AddTool("scale", "", new Runnable() {
            @Override
            public void run() {
                Scale();
            }
        });
        AddTool("mirror", "", new Runnable() {
            @Override
            public void run() {
                Mirror();
            }
        });
        AddTool("color", "", new Runnable() {
            @Override
            public void run() {
                Color();
            }
        });
        AddTool("layer", "", new Runnable() {
            @Override
            public void run() {
                Layer();
            }
        });
        AddTool("text", "", new Runnable() {
            @Override
            public void run() {
                Text();
            }
        });
        AddTool("path_text", "", new Runnable() {
            @Override
            public void run() {
                DrawTextAlongPath();
            }
        });
        AddTool("clean", "", new Runnable() {
            @Override
            public void run() {
                CleanDrawing();
            }
        });
        AddTool("slot", "", new Runnable() {
            @Override
            public void run() {
                Slot();
            }
        });
        AddTool("chamfer", "", new Runnable() {
            @Override
            public void run() {
                Chamfer();
            }
        });
        AddTool("fit", "", new Runnable() {
            @Override
            public void run() {
                Fit();
            }
        });
        AddTool("invert_delete", "", new Runnable() {
            @Override
            public void run() {
                InvertDelete();
            }
        });
        AddTool("remove_duplicates", "", new Runnable() {
            @Override
            public void run() {
                RemoveDuplicates();
            }
        });
        AddTool("lathe_mode", "", new Runnable() {
            @Override
            public void run() {
                LatheMode();
            }
        });
        AddTool("", "Tab", new Runnable() {
            @Override
            public void run() {
                Tab();
            }
        });
    }
    public void CheckKeyPress(String key)
    {
        for (int x = 0; x < ToolStack.size(); x++)
        {
            DrawingToolStruct tool = ToolStack.get(x);
            if (tool.hotkey.contentEquals(key))
            {
                tool.action.run();
                return;
            }
        }
    }
    public void CheckToolSeachInput(String input)
    {
        for (int x = 0; x < ToolStack.size(); x++)
        {
            DrawingToolStruct tool = ToolStack.get(x);
            if (tool.name.contentEquals(input))
            {
                tool.action.run();
                return;
            }
        }
    }
}
