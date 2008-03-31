package org.inkscape.script;

import java.awt.BorderLayout;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JTextPane;
import javax.swing.JScrollPane;
import javax.swing.text.DefaultCaret;
import javax.swing.text.Document;
import javax.swing.text.BadLocationException;
import javax.swing.text.StyleConstants;
import javax.swing.text.SimpleAttributeSet;
import java.awt.Color;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.Font;

import java.io.ByteArrayOutputStream;
import java.io.PrintStream;
import java.io.IOException;
import java.io.Writer;
import java.io.PrintWriter;

public class Terminal extends JPanel
                      implements KeyListener
{

SimpleAttributeSet inTextAttr;
SimpleAttributeSet outTextAttr;
SimpleAttributeSet errTextAttr;

StringBuffer buf = new StringBuffer();
JTextPane textPane;

void err(String msg)
{
    System.out.println("Terminal err: " + msg);
}

void trace(String msg)
{
    System.out.println("Terminal: " + msg);
}


void processInputLine(String txt)
{
    ScriptConsole cons = ScriptConsole.getInstance();
    if (cons != null)
        cons.doRunCmd(txt);
}



class OutWriter extends Writer
{

public void write(char[] cbuf, int off, int len)
{
    String s = new String(cbuf, off, len);
    output(s);
}

public void flush()
{
}

public void close()
{
}

}



PrintWriter outWriter;

public Writer getOutWriter()
{
    if (outWriter == null)
        outWriter = new PrintWriter(new OutWriter());
    return outWriter;
}


public void output(String txt)
{
    Document doc = textPane.getDocument();
    try
        {
        doc.insertString(doc.getLength(), txt, outTextAttr);
        textPane.setCaretPosition(doc.getLength());
        }
    catch (BadLocationException e)
        {
        }

}


public void outputf(String fmt, Object... args)
{
    ByteArrayOutputStream baos = new ByteArrayOutputStream();
    PrintStream out = new PrintStream(baos);
    out.printf(fmt, args);
    out.close();
    String s = baos.toString();
    output(s);
}



class ErrWriter extends Writer
{

public void write(char[] cbuf, int off, int len)
{
    String s = new String(cbuf, off, len);
    error(s);
}

public void flush()
{
}

public void close()
{
}

}



PrintWriter errWriter;

public Writer getErrWriter()
{
    if (errWriter == null)
        errWriter = new PrintWriter(new ErrWriter());
    return errWriter;
}


public void error(String txt)
{
    Document doc = textPane.getDocument();
    try
        {
        doc.insertString(doc.getLength(), txt, errTextAttr);
        textPane.setCaretPosition(doc.getLength());
        }
    catch (BadLocationException e)
        {
        }

}

public void errorf(String fmt, Object... args)
{
    ByteArrayOutputStream baos = new ByteArrayOutputStream();
    PrintStream out = new PrintStream(baos);
    out.printf(fmt, args);
    out.close();
    String s = baos.toString();
    error(s);
}


public void keyPressed(KeyEvent evt)
{
}

public void keyReleased(KeyEvent evt)
{
}

public void keyTyped(KeyEvent evt)
{
    Document doc = textPane.getDocument();
    char ch = evt.getKeyChar();
    if (ch == 127)
        {
        }
    else if (ch == '\b')
        {
        if (buf.length() == 0)
            return;
        try
            {
            buf.delete(buf.length()-1, buf.length());
            doc.remove(doc.getLength()-1, 1);
            textPane.setCaretPosition(doc.getLength());
            }
        catch (BadLocationException e)
            {
            err("keyTyped:" + e);
            }
        }
    else
        {
        try
            {
            buf.append(ch);
            doc.insertString(doc.getLength(), "" + ch, inTextAttr);
            textPane.setCaretPosition(doc.getLength());
            }
        catch (BadLocationException e)
            {
            }
        if (ch == '\n' || ch == '\r')
            {
            String txt = buf.toString();
            buf.delete(0, buf.length());
            txt = txt.trim();
            processInputLine(txt);
            }
        }



}



void setup()
{
    setLayout(new BorderLayout());
    textPane = new JTextPane();
    add(new JScrollPane(textPane), BorderLayout.CENTER);
    textPane.setEditable(false);
    textPane.setBackground(Color.BLACK);
    textPane.setCaretColor(Color.WHITE);
    textPane.setCaret(new DefaultCaret());
    textPane.getCaret().setVisible(true);
    textPane.getCaret().setBlinkRate(500);
    Font currentFont = textPane.getFont();
    textPane.setFont(new Font("Monospaced", currentFont.getStyle(), currentFont.getSize()));
    textPane.addKeyListener(this);

    inTextAttr = new SimpleAttributeSet();
    StyleConstants.setForeground(inTextAttr, Color.YELLOW);
    outTextAttr = new SimpleAttributeSet();
    StyleConstants.setForeground(outTextAttr, Color.GREEN);
    errTextAttr = new SimpleAttributeSet();
    StyleConstants.setForeground(errTextAttr, Color.RED);


}






public Terminal()
{
    super();
    setup();
}



public static void main(String argv[])
{
    Terminal t = new Terminal();
    JFrame par = new JFrame("Terminal Test");
    par.setSize(500, 350);
    par.getContentPane().add(t);
    par.setVisible(true);
}

}

