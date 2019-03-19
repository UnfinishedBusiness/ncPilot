package com.nc.pilot.dialogs;

import com.nc.pilot.config.JetToolpathCutChartData;
import com.nc.pilot.lib.GlobalData;

import javax.swing.*;
import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.DefaultTableModel;
import java.awt.*;
import java.awt.event.*;
import java.io.FileNotFoundException;
import java.util.ArrayList;

public class JetToolpathCutChart extends JFrame {
    private JPanel contentPane;
    private JButton buttonOK;
    private JButton buttonCancel;
    private JButton AddButton;
    private JButton RemoveButton;
    private JButton MoveUp;
    private JButton MoveDown;
    private JTable table1;
    private JScrollPane scrollPane;

    public JetToolpathCutChart() {

        //headers for the table
        table1 = new JTable();
        String[] columns = new String[] {
                "Material", "Consumable", "Amperage", "Kerf Diam.", "Pierce Height", "Pierce Delay", "Cut Height", "Post Delay", "ATHC Voltage", "Feedrate"
        };
        //create table with data
        DefaultTableModel dtm = new DefaultTableModel(0, 0);
        dtm.setColumnIdentifiers(columns);
        table1.setModel(dtm);
        for (int x = 0; x < GlobalData.configData.CutChart.size(); x++)
        {
            dtm.addRow(new Object[]{GlobalData.configData.CutChart.get(x).Material, GlobalData.configData.CutChart.get(x).Consumable, GlobalData.configData.CutChart.get(x).Amperage, GlobalData.configData.CutChart.get(x).KerfDiameter, GlobalData.configData.CutChart.get(x).PierceHeight, GlobalData.configData.CutChart.get(x).PierceDelay, GlobalData.configData.CutChart.get(x).CutHeight, GlobalData.configData.CutChart.get(x).PostDelay, GlobalData.configData.CutChart.get(x).ATHCVoltage, GlobalData.configData.CutChart.get(x).Feedrate});
        }
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

        AddButton = new JButton();
        AddButton.setText("Add");

        RemoveButton = new JButton();
        RemoveButton.setText("Remove");

        MoveUp = new JButton();
        MoveUp.setText("Move Up");

        MoveDown = new JButton();
        MoveDown.setText("Move Down");

        contentPane.add(MoveUp);
        contentPane.add(MoveDown);
        contentPane.add(AddButton);
        contentPane.add(RemoveButton);
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

        AddButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                JetToolpathCutChartData cut = new JetToolpathCutChartData();
                cut.Material = "New Material";
                cut.Consumable = "45A Finecut";
                cut.Amperage = 38;
                cut.KerfDiameter = 0.039f;
                cut.PierceHeight = 0.150f;
                cut.PierceDelay = 1.2f;
                cut.CutHeight = 0.175f;
                cut.PostDelay = 1f;
                cut.ATHCVoltage = 100;
                cut.Feedrate = 52f;
                GlobalData.configData.CutChart.add(cut);
                dtm.addRow(new Object[]{cut.Material, cut.Consumable, cut.Amperage, cut.KerfDiameter, cut.PierceHeight, cut.PierceDelay, cut.CutHeight, cut.PostDelay, cut.ATHCVoltage, cut.Feedrate});
                try {
                    GlobalData.pushConfig();
                } catch (FileNotFoundException e1) {
                    e1.printStackTrace();
                }
            }
        });
        RemoveButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                int row = table1.getSelectedRow();
                //System.out.println("Selected row: " + row);
                if (row > 0)
                {
                    ArrayList<JetToolpathCutChartData> tmp = new ArrayList();
                    for (int x = 0; x < GlobalData.configData.CutChart.size(); x++)
                    {
                        if (x != row)
                        {
                            tmp.add(GlobalData.configData.CutChart.get(x));
                        }
                    }
                    GlobalData.configData.CutChart = tmp;
                    dtm.removeRow(row);
                    try {
                        GlobalData.pushConfig();
                    } catch (FileNotFoundException e1) {
                        e1.printStackTrace();
                    }
                }
            }
        });
        MoveUp.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                int row = table1.getSelectedRow();
                //System.out.println("Selected row: " + row);
                if (row > 0)
                {
                    dtm.moveRow(row, row, row-1);
                    GlobalData.configData.CutChart = new ArrayList();
                    for (int x = 0; x < dtm.getRowCount(); x++)
                    {
                        JetToolpathCutChartData cut = new JetToolpathCutChartData();
                        cut.Material = dtm.getValueAt(x, 0).toString();
                        cut.Consumable = dtm.getValueAt(x, 1).toString();
                        cut.Amperage = new Integer(dtm.getValueAt(x, 2).toString());
                        cut.KerfDiameter = new Float(dtm.getValueAt(x, 3).toString());;
                        cut.PierceHeight = new Float(dtm.getValueAt(x, 4).toString());;
                        cut.PierceDelay = new Float(dtm.getValueAt(x, 5).toString());;
                        cut.CutHeight = new Float(dtm.getValueAt(x, 6).toString());;
                        cut.PostDelay = new Float(dtm.getValueAt(x, 7).toString());;
                        cut.ATHCVoltage = new Integer(dtm.getValueAt(x, 8).toString());;
                        cut.Feedrate = new Float(dtm.getValueAt(x, 9).toString());;
                        GlobalData.configData.CutChart.add(cut);
                    }
                    try {
                        GlobalData.pushConfig();
                    } catch (FileNotFoundException e1) {
                        e1.printStackTrace();
                    }
                }
            }
        });
        MoveDown.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                int row = table1.getSelectedRow();
                //System.out.println("Selected row: " + row);
                if (row > 0)
                {
                    dtm.moveRow(row, row, row+1);
                    GlobalData.configData.CutChart = new ArrayList();
                    for (int x = 0; x < dtm.getRowCount(); x++)
                    {
                        JetToolpathCutChartData cut = new JetToolpathCutChartData();
                        cut.Material = dtm.getValueAt(x, 0).toString();
                        cut.Consumable = dtm.getValueAt(x, 1).toString();
                        cut.Amperage = new Integer(dtm.getValueAt(x, 2).toString());
                        cut.KerfDiameter = new Float(dtm.getValueAt(x, 3).toString());;
                        cut.PierceHeight = new Float(dtm.getValueAt(x, 4).toString());;
                        cut.PierceDelay = new Float(dtm.getValueAt(x, 5).toString());;
                        cut.CutHeight = new Float(dtm.getValueAt(x, 6).toString());;
                        cut.PostDelay = new Float(dtm.getValueAt(x, 7).toString());;
                        cut.ATHCVoltage = new Integer(dtm.getValueAt(x, 8).toString());;
                        cut.Feedrate = new Float(dtm.getValueAt(x, 9).toString());;
                        GlobalData.configData.CutChart.add(cut);
                    }
                    try {
                        GlobalData.pushConfig();
                    } catch (FileNotFoundException e1) {
                        e1.printStackTrace();
                    }
                }
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
