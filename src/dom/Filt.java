

import java.io.*;

public class Filt
{

void p(String s)
{
    System.out.println(s);
}

void output(String s)
{
    String name = s.trim();
    if (name == null || name.length()<2)
        return;
    String ucName = name.substring(0,1).toUpperCase() +
                    name.substring(1);
        
    p("/**");
    p(" *  return the '" + name + "' property" );
    p(" */");
    p("DOMString CSS2PropertiesImpl::get" + ucName + "()");
    p("{");
    p("    return " + name + ";");
    p("}");
    p("");
    p("/**");
    p(" *  set the '" + name + "' property");
    p(" */");
    p("void CSS2PropertiesImpl::set" + ucName + "(const DOMString &val)");
    p("                         throw (dom::DOMException)");
    p("{");
    p("    " + name + " = val;");
    p("}");
    p("");


}


void doIt()
{
    try
        {
        BufferedReader in = new BufferedReader(new FileReader("cssprop.txt"));
    
        while (true)
            {
            String s = in.readLine();
            if (s == null)
                break;
            output(s);
            }
        
        in.close();
        }
    catch (Exception e)
        {
        }

}



public static void main(String argv[])
{
    Filt f = new Filt();
    f.doIt();

}



}

