package org.inkscape.script;



import javax.swing.JPanel;
import javax.swing.JTextPane;
import java.awt.BorderLayout;

/**
 * A simple script editor for quick fixes.
 */
public class Editor extends JPanel
{
JTextPane textPane;


/**
 *
 */
public String getText()
{
    return textPane.getText();
}


/**
 *
 */
public void setText(String txt)
{
    textPane.setText(txt);
}


/**
 *
 */
public Editor()
{
    super();
    setLayout(new BorderLayout());
    textPane = new JTextPane();
    add(textPane, BorderLayout.CENTER);
}



}

