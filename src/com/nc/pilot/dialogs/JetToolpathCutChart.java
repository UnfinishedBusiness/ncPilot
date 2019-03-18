package com.nc.pilot.dialogs;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

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
        Object[][] data = new Object[][] {
                {"1/8 Mild Steel", "45A Finecut", 38, 0.039f, 0.150f, 1.2f, 0.175f, 1, 100, 52},
                {"1/4 Mild Steel", "45A Finecut", 45, 0.042f, 0.150f, 1.8f, 0.175f, 1, 100, 35},
                {"3/8 Mild Steel", "45A Finecut", 45, 0.048f, 0.150f, 1.8f, 0.175f, 1, 100, 28},
        };
        //create table with data
        table1 = new JTable(data, columns);
        table1.setPreferredScrollableViewportSize(table1.getPreferredSize());
        table1.setFillsViewportHeight(true);
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
