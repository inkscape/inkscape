import java.io.*;
import java.util.*;


public class Idlp
{
String parsebuf;
int len;


void error(String msg)
{
    System.out.println("Idlp err : " + msg);
}

void trace(String msg)
{
    System.out.println("Idlp : " + msg);
}






int get(int pos)
{
    if (pos<0 || pos>=len)
        return -1;
    else
        return (int) parsebuf.charAt(pos);
}


int skipwhite(int pos)
{
    trace("skipwhite()");
    while (pos < len)
        {
        int ch = get(pos);
        if (ch == '/' && get(pos + 1) == '/')
            {
            pos += 2;
            while (pos < len)
                {
                ch = get(pos);
                if (ch == '\n' || ch == '\r')
                    break;
                pos++;
                }
            }
        else if (!Character.isWhitespace(ch))
            {
            break;
            }
        pos++;
        }
    return pos;
}

boolean match(String key)
{
    trace("match(" + key + ")");
    int p = 0;
    for (int i=0 ; i<key.length() ; i++)
        {
        if (get(p) != key.charAt(i))
            return false;
        p++;
        }
    return true;
        
}

String word = "";

int getword(int pos)
{
    trace("getword()");
    StringBuffer buf = new StringBuffer();
    while (pos < len)
        {
        int ch = get(pos);
        if (ch < 0)
            break;
        if (!Character.isLetterOrDigit(ch) && ch != '#' && ch != '_')
            break;
        buf.append((char)ch);
        pos++;
        }
    word = buf.toString();
    return pos;
}

int getDirective(String name, int pos)
{
    trace("getDirective()");
    if (name.length() == 0 || name.charAt(0) != '#')
        return -1;
    while (pos < len)
        {
        int ch = get(pos);
        if (ch == '\n' || ch == '\r')
            break;
        pos++;
        }
    return pos;
}

int getTypedef(int pos)
{
    trace("getTypedef()");
    while (pos < len)
        {
        int ch = get(pos++);
        if (ch == ';')
            break;
        }
    return pos;
}


int getInterface(int pos)
{
    trace("getInterface()");
    pos = skipwhite(pos);
    int p = getword(pos);
    if (p < 0)
        return -1;
    if (p <= pos)
        {
        error("expected interface name");
        return -1;
        }
    String intfName = word;
    trace("intf: " + intfName);
    pos = p;
    pos = skipwhite(pos);
    int ch = get(pos);
    if (ch == ';')
        {
        pos++; //forward decl
        trace("forward decl");
        return pos;
        }
    if (ch != '{')
        {
        error("Expected opening { for interface");
        return -1;
        }
    pos++;
    while (true)
        {
        pos = skipwhite(pos);
        ch = get(pos);
        if (ch == '}')
            {
            break;
            }
        p = getword(pos);
        if (p < 0)
            {
            return -1;
            }
        if (p<=pos)
            {
            error("expected word");
            return -1;
            }
        trace("word : " + word);
        if (word.equals("typedef"))
            {
            pos = p;
            p = getTypedef(pos);
            if (p < 0)
                return -1;
            }
        pos = p;
        }

    return pos;
}


int getException(int pos)
{
    trace("getException()");
    pos = skipwhite(pos);
    int p = getword(pos);
    if (p < 0)
        return -1;
    if (p <= pos)
        {
        error("expected exception name");
        return -1;
        }
    String exName = word;
    trace("ex: " + exName);
    pos = p;
    pos = skipwhite(pos);
    int ch = get(pos);
    if (ch == ';')
        {
        pos++; //forward decl
        trace("forward decl");
        return pos;
        }
    if (ch != '{')
        {
        error("Expected opening { for exception");
        return -1;
        }
    pos++;
    while (pos < len)
        {
        ch = get(pos++);
        if (ch == '}')
            {
            break;
            }
        }
    pos = skipwhite(pos);
    ch = get(pos);
    if (ch != ';')
        {
        error("expected ; for exception");
        return -1;
        }
    pos++;
    return pos;
}



int getModule(int pos)
{
    trace("getModule()");
    pos = skipwhite(pos);
    int p = getword(pos);
    if (p < 0)
        return -1;
    if (p <= pos)
        {
        error("expected module name");
        return -1;
        }
    String modName = word;
    trace("mod: " + modName);
    pos = p;
    pos = skipwhite(pos);
    int ch = get(pos);
    if (ch != '{')
        {
        error("Expected opening { for module");
        return -1;
        }
    pos++;
    while (true)
        {
        pos = skipwhite(pos);
        ch = get(pos);
        if (ch == '}')
            {
            break;
            }
        p = getword(pos);
        if (p < 0)
            {
            return -1;
            }
        if (p<=pos)
            {
            error("expected word");
            return -1;
            }
        trace("word : " + word);
        if (word.equals("typedef"))
            {
            pos = p;
            p = getTypedef(pos);
            if (p < 0)
                return -1;
            }
        else if (word.equals("interface"))
            {
            pos = p;
            p = getInterface(pos);
            if (p < 0)
                return -1;
            }
        else if (word.equals("exception"))
            {
            pos = p;
            p = getException(pos);
            if (p < 0)
                return -1;
            }
        else if (word.equals("module"))
            {
            pos = p;
            p = getModule(pos);
            if (p < 0)
                return -1;
            }
        pos = p;
        }

    return pos;
}

boolean parse()
{
    trace("parse()");
    len = parsebuf.length();
    int pos = 0;
    while (pos < len)
        {
        pos = skipwhite(pos);
        if (pos >= len)
            break;
        int ch = get(pos);
        int p = getword(pos);
        if (p < 0)
            return false;
        if (p<=pos)
            {
            error("expected word");
            return false;
            }
        trace("word: " + word);
        if (word.length() == 0)
            break;
        if (word.charAt(0) == '#')
            {
            p = getDirective(word, pos);
            if (p < 0)
                return false;
            }
        else if (word.equals("module"))
            {
            pos = p;
            p = getModule(pos);
            if (p<0)
                return false;
            }
        pos = p;
        }

    return true;
}


boolean run()
{
    parsebuf = "";
    boolean ret = true;
    try
        {
        StringBuffer inbuf = new StringBuffer();
        FileReader in = new FileReader("svg.idl");
        while (true)
            {
            int ch = in.read();
            if (ch < 0)
                break;
            inbuf.append((char)ch);
            }
        in.close();
        parsebuf = inbuf.toString();
        ret = parse();
        }
    catch (IOException e)
        {
        error("run : " + e);
        return false;
        }
    return ret;
}


public Idlp()
{
}


public static void main(String argv[])
{
    Idlp idlp = new Idlp();
    idlp.run();
}



}
