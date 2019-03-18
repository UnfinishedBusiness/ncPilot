package com.nc.pilot.dialogs;

import com.nc.pilot.lib.GlobalData;

import javax.swing.*;
import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import java.awt.*;
import java.awt.event.*;
import java.io.FileNotFoundException;
import java.util.ArrayList;

public class JetToolpathJobSetup extends JFrame {

    public JetToolpathJobSetup() {

        JFrame frame = new JFrame("Job Setup");

        JComboBox material_selection = new JComboBox();
        ArrayList<String> material_list = new ArrayList();
        for (int x = 0; x < GlobalData.configData.CutChart.size(); x++)
        {
            boolean found_material = false;
            for (int y = 0; y < material_list.size(); y++)
            {
                if (material_list.get(y).contentEquals(GlobalData.configData.CutChart.get(x).Material))
                {
                    found_material = true;
                    break;
                }
            }
            if (found_material == false)
            {
                material_list.add(GlobalData.configData.CutChart.get(x).Material);
            }
        }
        for (int x = 0; x < material_list.size(); x++) material_selection.addItem(material_list.get(x));
        material_selection.setSelectedItem(GlobalData.configData.MaterialSelection);
        frame.add(material_selection);

        JComboBox consumable_selection = new JComboBox();
        ArrayList<String> consumable_list = new ArrayList();
        for (int x = 0; x < GlobalData.configData.CutChart.size(); x++)
        {
            boolean found_material = false;
            for (int y = 0; y < consumable_list.size(); y++)
            {
                if (consumable_list.get(y).contentEquals(GlobalData.configData.CutChart.get(x).Consumable))
                {
                    found_material = true;
                    break;
                }
            }
            if (found_material == false)
            {
                consumable_list.add(GlobalData.configData.CutChart.get(x).Consumable);
            }
        }
        for (int x = 0; x < consumable_list.size(); x++) consumable_selection.addItem(consumable_list.get(x));
        consumable_selection.setSelectedItem(GlobalData.configData.ConsumableSelection);
        frame.add(consumable_selection);


        JButton ok_button;
        ok_button = new JButton("OK");
        frame.add(ok_button);
        ok_button.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                GlobalData.configData.MaterialSelection = material_selection.getSelectedItem().toString();
                GlobalData.configData.ConsumableSelection = consumable_selection.getSelectedItem().toString();
                try {
                    GlobalData.pushConfig();
                } catch (FileNotFoundException e1) {
                    e1.printStackTrace();
                }
                dispose();
            }
        });


        frame.setLayout(new FlowLayout());
        frame.setSize(300,300);
        frame.setVisible(true);
        frame.setLocationRelativeTo(null);
        add(frame);
        setDefaultCloseOperation(DO_NOTHING_ON_CLOSE);
    }

    public static void main(String[] args) {
        JetToolpathJobSetup dialog = new JetToolpathJobSetup();
        dialog.pack();
        dialog.setSize(550, 475);
        dialog.setVisible(true);
    }
}
