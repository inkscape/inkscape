

import java.io.*;
import java.util.StringTokenizer;



public class ImplGen
{
BufferedWriter out;
String         className;
String         defaultReturn;

void trace(String msg)
{
    System.out.println(msg);
}

void err(String msg)
{
    System.out.print("error:");
    System.out.println(msg);
}

void p(String s)
{
    try
        {
        out.write(s);
        }
    catch (IOException e)
        {
        }
}

void sp(int count)
{
    for (int i=0 ; i<count ; i++)
        p(" ");
}



void process(String s)
{

    if (s.startsWith("## "))
        {
        className = s.substring(3);
        }

    if (s.startsWith("DOMString"))
        {
        defaultReturn = "DOMString(\"\")";
        }
    else if (s.startsWith("bool"))
        {
        defaultReturn = "false";
        }
    else if (s.startsWith("void"))
        {
        defaultReturn = "";
        }
    else if (s.startsWith("unsigned long") || s.startsWith("long"))
        {
        defaultReturn = "0L";
        }
    else if (s.startsWith("unsigned short") || s.startsWith("short"))
        {
        defaultReturn = "0";
        }
    else if (s.startsWith("float") || s.startsWith("double"))
        {
        defaultReturn = "0.0";
        }
    else if (s.length()>0 && Character.isLetter(s.charAt(0)))
        {
        defaultReturn = "NULL";
        }
    else if (s.startsWith("~"))
        {
        defaultReturn = "";
        }

    int pos = s.indexOf("(");
    if (!s.startsWith("~") && pos > 0 &&
            Character.isLetterOrDigit(s.charAt(pos-1)))
        {
        while (Character.isLetterOrDigit(s.charAt(pos-1)))
            pos--;
        String news = s.substring(0, pos) +
                        className + "::" +
                      s.substring(pos);
        s = news;
        }

    if (s.startsWith("~"))
        {
        p(className); p("::");
        p(s); p("\n");
        }
    else if (s.startsWith("}") && defaultReturn.length()>0)
        {
        p("    return "); p(defaultReturn); p(";"); p("\n");
        p(s); p("\n");
        }
    else
        {
        p(s); p("\n");
        }



}





void doIt(String inName)
{
    String cppName = inName + ".cpp";
    try
        {
        BufferedReader in = new BufferedReader(new FileReader(inName));
        out = new BufferedWriter(new FileWriter(cppName));
        while (true)
            {
            String s = in.readLine();
            if (s == null)
                break;
            process(s);
            }
        
        in.close();
        out.close();
        }
    catch (Exception e)
        {
        }

}


public ImplGen()
{
}


public static void main(String argv[])
{
    if (argv.length != 1)
        {
        System.out.println("usage: ImplGen <source .h file>");
        return;
        }
    ImplGen ig = new ImplGen();
    ig.doIt(argv[0]);

}



















}
