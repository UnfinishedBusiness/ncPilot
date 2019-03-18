package com.nc.pilot.dialogs;

import com.nc.pilot.config.JetToolpathCutChartData;
import com.nc.pilot.lib.GlobalData;

import javax.swing.*;
import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import java.awt.*;
import java.awt.event.*;
import java.io.FileNotFoundException;

public class JetToolpathCutChart extends JFrame {
    private JPanel contentPane;
    private JButton buttonOK;
    private JButton buttonCancel;
    private JTable table1;
    private JScrollPane scrollPane;

    public JetToolpathCutChart() {

        //headers for the table
        String[] columns = new String[] {
                "Material", "Consumable", "Amperage", "Kerf Diam.", "Pierce Height", "Pierce Delay", "Cut Height", "Post Delay", "ATHC Voltage", "Feedrate"
        };
        //actual data for the table in a 2d array
        /*Object[][] data = new Object[][] {
                {"1/8 Mild Steel", "45A Finecut", 38, 0.039f, 0.150f, 1.2f, 0.175f, 1, 100, 52},
                {"1/4 Mild Steel", "45A Finecut", 45, 0.042f, 0.150f, 1.8f, 0.175f, 1, 100, 35},
                {"3/8 Mild Steel", "45A Finecut", 45, 0.048f, 0.150f, 1.8f, 0.175f, 1, 100, 28},
        };*/

        Object[][] data = new Object[GlobalData.configData.CutChart.size()][10];
        for (int x = 0; x < GlobalData.configData.CutChart.size(); x++)
        {
            data[x][0] = GlobalData.configData.CutChart.get(x).Material;
            data[x][1] = GlobalData.configData.CutChart.get(x).Consumable;
            data[x][2] = GlobalData.configData.CutChart.get(x).Amperage;
            data[x][3] = GlobalData.configData.CutChart.get(x).KerfDiameter;
            data[x][4] = GlobalData.configData.CutChart.get(x).PierceHeight;
            data[x][5] = GlobalData.configData.CutChart.get(x).PierceDelay;
            data[x][6] = GlobalData.configData.CutChart.get(x).CutHeight;
            data[x][7] = GlobalData.configData.CutChart.get(x).PostDelay;
            data[x][8] = GlobalData.configData.CutChart.get(x).ATHCVoltage;
            data[x][9] = GlobalData.configData.CutChart.get(x).Feedrate;
        }
        //create table with data
        table1 = new JTable(data, columns);
        table1.setPreferredScrollableViewportSize(table1.getPreferredSize());
        table1.setFillsViewportHeight(true);
        table1.getModel().addTableModelListener(new TableModelListener() {

            public void tableChanged(TableModelEvent e) {
                // your code goes here, whatever you want to do when something changes in the table
                int row = table1.getSelectedRow();
                //int column = table1.getSelectedColumn();
                //System.out.println(table1.getValueAt(table1.getSelectedRow(),table1.getSelectedColumn()));
                if (row < GlobalData.configData.CutChart.size())
                {
                    GlobalData.configData.CutChart.get(row).Material = (String)table1.getValueAt(row, 0);
                    GlobalData.configData.CutChart.get(row).Consumable = (String)table1.getValueAt(row, 1);
                    GlobalData.configData.CutChart.get(row).Amperage = new Integer(table1.getValueAt(row, 2).toString());
                    GlobalData.configData.CutChart.get(row).KerfDiameter = new Float(table1.getValueAt(row, 3).toString());
                    GlobalData.configData.CutChart.get(row).PierceHeight = new Float(table1.getValueAt(row, 4).toString());
                    GlobalData.configData.CutChart.get(row).PierceDelay = new Float(table1.getValueAt(row, 5).toString());
                    GlobalData.configData.CutChart.get(row).CutHeight = new Float(table1.getValueAt(row, 6).toString());
                    GlobalData.configData.CutChart.get(row).PostDelay = new Float(table1.getValueAt(row, 7).toString());
                    GlobalData.configData.CutChart.get(row).ATHCVoltage = new Integer(table1.getValueAt(row, 8).toString());
                    GlobalData.configData.CutChart.get(row).Feedrate = new Float(table1.getValueAt(row, 9).toString());
                    try {
                        GlobalData.pushConfig();
                    } catch (FileNotFoundException e1) {
                        e1.printStackTrace();
                    }
                }
            }
        });
        scrollPane = new JScrollPane(table1);
        scrollPane.setPreferredSize(new Dimension(800, 400));

        contentPane = new JPanel();
        contentPane.add(scrollPane);

        buttonOK = new JButton();
        buttonOK.setText("OK");

        buttonCancel = new JButton();
        buttonCancel.setText("Cancel");

        contentPane.add(buttonCancel);
        contentPane.add(buttonOK);

        add(contentPane);
        getRootPane().setDefaultButton(buttonOK);

        buttonOK.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                onOK();
            }
        });

        buttonCancel.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                onCancel();
            }
        });

        // call onCancel() when cross is clicked
        setDefaultCloseOperation(DO_NOTHING_ON_CLOSE);
        addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent e) {
                onCancel();
            }
        });

        // call onCancel() on ESCAPE
        contentPane.registerKeyboardAction(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                onCancel();
            }
        }, KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0), JComponent.WHEN_ANCESTOR_OF_FOCUSED_COMPONENT);
    }

    private void onOK() {
        // add your code here
        dispose();
    }

    private void onCancel() {
        // add your code here if necessary
        dispose();
    }

    public static void main(String[] args) {
        JetToolpathCutChart dialog = new JetToolpathCutChart();
        dialog.pack();
        dialog.setSize(820, 475);
        dialog.setLocationRelativeTo(null);
        dialog.setVisible(true);
    }
}
