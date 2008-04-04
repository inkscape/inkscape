
import java.io.*;
import java.util.HashMap;
import java.util.ArrayList;
import java.util.Collections;


/**
 * This is not an actual java binding class.  Rather, it is
 * a simple tool for generating C++ native method stubs from classfiles
 */  
public class DomStub
{

class MethodEntry
{
ArrayList<String> parms;
String name;
String type;

public void addParam(String type)
{
    parms.add(type);
}

public MethodEntry(String methodName, String type)
{
    this.name = methodName;
    this.type = type;
    parms = new ArrayList<String>();
}
}


class ClassEntry
{
ArrayList<MethodEntry> methods;
String name;

public void addMethod(MethodEntry method)
{
    methods.add(method);
}

public ClassEntry(String className)
{
    this.name = className;
    methods = new ArrayList<MethodEntry>();
}
}


HashMap<String, ClassEntry> classes;


BufferedWriter out;

void err(String msg)
{
    System.out.println("DomStub err:" + msg);
}

void trace(String msg)
{
    System.out.println("DomStub:" + msg);
}

void po(String msg)
{
    try
	    {
		out.write(msg);
		}
	catch (IOException e)
	    {
		}
}


//########################################################################
//# G E N E R A T E
//########################################################################

void dumpClasses()
{
    for (ClassEntry ce : classes.values())
        {
        trace("########################");
        trace("Class " + ce.name);
        for (MethodEntry me : ce.methods)
            {
            trace("    " + me.type + " " + me.name);
            for (String parm : me.parms)
               {
               trace("        " + parm);
               }
			}
		}
}


void generateMethod(MethodEntry me)
{
    po("/**\n");
    po(" *  Method : " + me.name + "\n");
    po(" */\n");
    for (String parm : me.parms)
        {
        po("        " + parm + "\n");
        }

}

void generateClass(ClassEntry ce)
{
    po("//################################################################\n");
    po("//##  " + ce.name + "\n");
    po("//################################################################\n");

    for (MethodEntry me : ce.methods)
        generateMethod(me);

}


void generate()
{
    ArrayList<String> classNames = new ArrayList<String>(classes.keySet());
    Collections.sort(classNames);
    for (String key  : classNames)
        {
        ClassEntry ce = classes.get(key);
        generateClass(ce);
        }
}

//########################################################################
//# P A R S E
//########################################################################
boolean parseEntry(String className, String methodName, String signature)
{
    //trace("Decl  :" + methodDecl);
    //trace("params:" + params);
    //#################################
    //# Parse class and method lines
    //#################################
    String s = className.substring(14);
    className = s.replace('_', '/');
    methodName = methodName.substring(14);
    signature = signature.substring(14);
    //trace("className   : " + className);
    //trace("methodName  : " + methodName);

    int pos = signature.indexOf('(');
	if (pos<0)
	    {
		err("no opening ( for signature");
		return false;
		}
	pos++;
    int p2 = signature.indexOf(')', pos);
    if (p2<0)
	    {
		err("no closing ) for signature");
		return false;
		}
	String parms = signature.substring(pos, p2);
	String type  = signature.substring(p2+1);
    //#################################
    //# create method entry.  add to new or existing class
    //#################################
    MethodEntry method = new MethodEntry(methodName, type);
    
	ClassEntry clazz = classes.get(className);
	if (clazz == null)
	    {
	    clazz = new ClassEntry(className);
	    classes.put(className, clazz);
		}
	clazz.addMethod(method);

    //#################################
    //# Parse signature line
    //#################################
    
    pos = 0;
    int len = parms.length();
    while (pos<len)
        {
        String typ = "";
        char ch = parms.charAt(pos);
        if (ch == '[')
            {
			pos++;
            ch = parms.charAt(pos);
            typ += ch;
            }
        if (ch == 'L')
            {
            pos++;
            typ += ch;
            while (pos<len)
                {
                ch = parms.charAt(pos);
                if (ch == ';')
                    break;
                typ += ch;
                pos++;
				}
			}
		else
		    typ += ch;
		method.addParam(typ);
		//trace("param:" + typ);
    	pos++;
		}
    return true;
}


public boolean parseFile(String fname)
{
    boolean ret = true;
    try
        {
		BufferedReader in = new BufferedReader(new FileReader(fname));
		while (true)
		    {
			String s1 = in.readLine();
			if (s1 == null)
			    break;
			if (!s1.startsWith(" * Class:     "))
			    continue;
			String s2 = in.readLine();
			if (!s2.startsWith(" * Method:    "))
			    continue;
			String s3 = in.readLine();
			if (!s3.startsWith(" * Signature: "))
			    continue;
			if (!parseEntry(s1, s2, s3))
			    {
			    ret = false;
			    break;
				}
			}
		in.close();
		}
	catch (IOException e)
	    {
	    err("processFile:" + e);
	    ret  = false;
		}
    return ret;
}


public boolean processFile(String fname)
{
    try
        {
		out = new BufferedWriter(new FileWriter("out.txt"));
		}
	catch (IOException e)
	    {
	    err("processFile: " + e);
	    return false;
		}
    if (!parseFile(fname))
        return false;
    //dumpClasses();
    generate();
    try
        {
		out.close();
		}
	catch (IOException e)
	    {
	    err("processFile: " + e);
	    return false;
		}
    return true;
}


public DomStub()
{
    classes = new HashMap<String, ClassEntry>();
}



public static void main(String argv[])
{
    DomStub st = new DomStub();
    boolean ret = st.processFile("out.h");
}


}