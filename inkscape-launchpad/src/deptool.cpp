/*
 * DepTool dependency tool
 *
 * This is a simple dependency generator coded in C++
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2006 Bob Jamison
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/*
 INSTRUCTIONS

 1.  First you optionally create a file named 'make.exclude.'  This
 lists the file names relative to the current directory.  Lines starting
 with a '#' are ignored.  Here is an example:
 
========= SNIP ======== 
######################################################################
# File: make.exclude
#
# This is a list of files to exclude from building, using DepTool
#
######################################################################


ast
bonobo

dialogs/filedialog-win32.cpp
display/testnr.cpp
display/bezier-utils-test.cpp
dom/work
#next line is ignored
#dom/odf/SvgOdg.cpp
extension/api.cpp
========= SNIP ======== 

2.  Run deptool.  This will take a few seconds to scan all of your files
and make its dependency lists.  Three files are created:
make.files: lists all of the files that were considered
make.dep: contains the output INCLUDE and OBJECT declarations
    for you to include and use in your makefile.  It also contains
    the dependency listings.
make.ref: contains a reverse-dependency listing.  This takes each file
    and recursively determines which other files include it, and how
    far the nesting is.

3.  Include deptool.mk in your makefile.

*/


#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>

#include <dirent.h>
#include <sys/stat.h>

#include <string>
#include <set>
#include <map>
#include <vector>


//# This will allow us to redefine the string in the future
typedef std::string String;

/**
 *  Class which holds information for each file.
 */
class FileRec
{
public:

    typedef enum
        {
        UNKNOWN,
        CFILE,
        HFILE,
        OFILE
        } FileType;

    /**
     *  Constructor
     */
    FileRec()
        {type = UNKNOWN;}

    /**
     *  Copy constructor
     */
    FileRec(const FileRec &other)
        {assign(other);}
    /**
     *  Constructor
     */
    FileRec(int typeVal)
        {init(); type = typeVal;}
    /**
     *  Assignment operator
     */
    FileRec &operator=(const FileRec &other)
        {assign(other); return *this;}


    /**
     *  Destructor
     */
    ~FileRec()
        {}

    /**
     *  Directory part of the file name
     */
    String path;

    /**
     *  Base name, sans directory and suffix
     */
    String baseName;

    /**
     *  File extension, such as cpp or h
     */
    String suffix;

    /**
     *  Type of file: CFILE, HFILE, OFILE
     */
    int type;

    /**
     *  'Distance' of inclusion from a file that depends on this one.
     */
    int distance;

    /**
     * Used to list files ref'd by this one, in the case of allFiles,
     * or other files which reference this one, such as refFiles;
     */
    std::map<String, FileRec *> files;


    bool checked;

private:

    void init()
        {
        type    = UNKNOWN;
        checked = false;
        }

    void assign(const FileRec &other)
        {
        type     = other.type;
        distance = other.distance;
        baseName = other.baseName;
        suffix   = other.suffix;
        files    = other.files;
        checked  = other.checked;
        }

};



/**
 *  Main class which does the work.
 */
class DepTool
{
public:

    /**
     *  Constructor
     */
    DepTool();

    /**
     *  Destructor
     */
    virtual ~DepTool();

    /**
     * Creates the list of all file names which will be
     * candidates for further processing.  Reads make.exclude
     * to see which files for directories to leave out.
     */
    virtual bool createFileList();


    /**
     *  Generates the forward and reverse dependency lists
     */
    virtual bool generateDependencies();

    /**
     *  Calls the other two methods, then generates the files.
     */
    virtual bool run();


private:

    /**
     *
     */
    void reset();

    /**
     *
     */
    void error(char *fmt, ...);

    /**
     *
     */
    void trace(char *fmt, ...);

    /**
     *  Removes whitespace from beginning and end of a string
     */
    String trim(const String &val);

    /**
     *
     */
    String getSuffix(const String &fname);

    /**
     *
     */
    void parseName(const String &fullname,
                   String &path,
                   String &basename,
                   String &suffix);

    /**
     *
     */
    int get(int pos);

    /**
     *
     */
    int skipwhite(int pos);

    /**
     *
     */
    int getword(int pos, String &ret);

    /**
     *
     */
    bool sequ(int pos, char *key);

    /**
     *
     */
    bool listFilesInDirectory(const String &dirname, int depth);

    /**
     *
     */
    bool saveFileList();

    /**
     *
     */
    bool saveDepFile(bool doXml);

    /**
     *
     */
    bool saveCmakeFile();

    /**
     *
     */
    bool saveRefFile(bool doXml);

    /**
     *
     */
    bool addIncludeFile(FileRec *frec, const String &fname);

    /**
     *
     */
    bool scanFile(const String &fname, FileRec *frec);

    /**
     *
     */
    bool processDependency(FileRec *ofile,
                           FileRec *include,
                           int depth);

    /**
     *
     */
    bool processReference(FileRec *ofile,
                          String &fname,
                          int depth);

    /**
     *
     */
    std::set<String> excludes;

    /**
     *
     */
    std::set<String> excludesUsed;

    /**
     *
     */
    std::set<String> excludesUnused;

    /**
     *
     */
    std::vector<String> directories;

    /**
     * A list of all files which will be processed for
     * dependencies.
     */
    std::map<String, FileRec *> allFiles;

    /**
     * A list of .h files, with a list for each one
     * of which other files include them.
     */
    std::map<String, FileRec *> refFiles;

    /**
     * The list of .o files, and the
     * dependencies upon them.
     */
    std::map<String, FileRec *> depFiles;

    char *fileBuf;
    int   fileSize;
    
    static const int readBufLen = 8192;
    char readBuf[8193];


};


//########################################################################
//# M A I N
//########################################################################

/**
 * Constructor
 */
DepTool::DepTool()
{
}


/**
 *  Destructor
 */
DepTool::~DepTool()
{
    reset();
}

/**
 *  Clean up after processing.  Called by the destructor, but should
 *  also be called before the object is reused.
 */
void DepTool::reset()
{
    std::map<String, FileRec *>::iterator iter;
    for (iter=allFiles.begin() ; iter!=allFiles.end() ; iter++)
        {
        FileRec *frec = iter->second;
        delete frec;
        }
    allFiles.clear();
    for (iter=refFiles.begin() ; iter!=refFiles.end() ; iter++)
        {
        FileRec *frec = iter->second;
        delete frec;
        }
    refFiles.clear();
    for (iter=depFiles.begin() ; iter!=depFiles.end() ; iter++)
        {
        FileRec *frec = iter->second;
        delete frec;
        }
    depFiles.clear();

    excludes.clear();
    excludesUsed.clear();
    excludesUnused.clear();
}


/**
 *  Format an error message in printf() style
 */
void DepTool::error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "DepTool error: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
}


/**
 *  Format an trace message in printf() style
 */
void DepTool::trace(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stdout, "DepTool: ");
    vfprintf(stdout, fmt, ap);
    fprintf(stdout, "\n");
    va_end(ap);
}



//########################################################################
//# U T I L I T Y
//########################################################################

/**
 *  Removes whitespace from beginning and end of a string
 */
String DepTool::trim(const String &s)
{
    if (s.size() < 1)
        return s;
    
    //Find first non-ws char
    unsigned int begin = 0;
    for ( ; begin < s.size() ; begin++)
        {
        if (!isspace(s[begin]))
            break;
        }

    //Find first non-ws char, going in reverse
    unsigned int end = s.size() - 1;
    for ( ; end > begin ; end--)
        {
        if (!isspace(s[end]))
            break;
        }
    //trace("begin:%d  end:%d", begin, end);

    String res = s.substr(begin, end-begin+1);
    return res;
}


/**
 *  Parse a full path name into path, base name, and suffix
 */
void DepTool::parseName(const String &fullname,
                        String &path,
                        String &basename,
                        String &suffix)
{
    if (fullname.size() < 2)
        return;

    String::size_type pos = fullname.find_last_of('/');
    if (pos != fullname.npos && pos<fullname.size()-1)
        {
        path = fullname.substr(0, pos);
        pos++;
        basename = fullname.substr(pos, fullname.size()-pos);
        }
    else
        {
        path = "";
        basename = fullname;
        }

    pos = basename.find_last_of('.');
    if (pos != basename.npos && pos<basename.size()-1)
        {
        suffix   = basename.substr(pos+1, basename.size()-pos-1);
        basename = basename.substr(0, pos);
        }

    //trace("parsename:%s %s %s", path.c_str(),
    //        basename.c_str(), suffix.c_str()); 
}


/**
 *  Return the suffix, if any, of a file name
 */
String DepTool::getSuffix(const String &fname)
{
    if (fname.size() < 2)
        return "";
    String::size_type pos = fname.find_last_of('.');
    if (pos == fname.npos)
        return "";
    pos++;
    String res = fname.substr(pos, fname.size()-pos);
    //trace("suffix:%s", res.c_str()); 
    return res;
}



//########################################################################
//# P R O C E S S I N G
//########################################################################

/**
 *  Recursively list all files and directories under 'dirname', except
 *  those in the exclude list.
 */
bool DepTool::listFilesInDirectory(const String &dirname, int depth)
{
    //trace("### listFilesInDirectory(%s, %d)", dirname.c_str(), depth);

    int cFiles = 0;
    int hFiles = 0;

    std::vector<String> subdirs;
    DIR *dir = opendir(dirname.c_str());
    while (true)
        {
        struct dirent *de = readdir(dir);
        if (!de)
            break;
        String s = de->d_name;
        struct stat finfo;
        String fname;
        if (s.size() == 0 || s[0] == '.')
            continue;

        if (dirname.size()>0 && dirname != ".")
            {
            fname.append(dirname);
            fname.append("/");
            }
        fname.append(s);
        if (stat(fname.c_str(), &finfo)<0)
            {
            error("cannot stat file:%s", s.c_str());
            }
        else if (excludes.find(fname) != excludes.end())
            {
            //trace("excluded file/dir: %s", fname.c_str());
            excludesUsed.insert(fname);
            }
        else if (S_ISDIR(finfo.st_mode))
            {
            //trace("directory: %s", fname.c_str());
            subdirs.push_back(fname);
            }
        else if (!S_ISREG(finfo.st_mode))
            {
            trace("not regular: %s", fname.c_str());
            }
        else
            {
            String path;
            String basename;
            String sfx;
            parseName(fname, path, basename, sfx);
            if (sfx == "cpp" || sfx == "c" || sfx == "cxx"   || sfx == "cc")
                {
                cFiles++;
                FileRec *fe  = new FileRec(FileRec::CFILE);
                fe->path     = path;
                fe->baseName = basename;
                fe->suffix   = sfx;
                allFiles[fname] = fe;
                }
            else if (sfx == "h"   ||  sfx == "hh"  ||
                     sfx == "hpp" ||  sfx == "hxx")
                {
                hFiles++;
                FileRec *fe = new FileRec(FileRec::HFILE);
                fe->path     = path;
                fe->baseName = basename;
                fe->suffix   = sfx;
                allFiles[fname] = fe;
                }
            }
        }
    closedir(dir);

    if (hFiles > 0)
        directories.push_back(dirname);

    for (unsigned int i=0 ; i<subdirs.size() ; i++)
        {
        listFilesInDirectory(subdirs[i], depth+1);
        }

    return true;
}



/**
 *
 */
bool DepTool::createFileList()
{
    excludes.insert("deptool.cpp");
    FILE *f = fopen("make.exclude", "r");
    if (!f)
        {
        trace("'make.exclude not found");
        }
    else
        {
        char readBuf[256];
        while (!feof(f))
            {
            if (!fgets(readBuf, 255, f))
                break;
            String s = readBuf;
            s = trim(s);
            //trace("s: %d '%s' ", s.size(), s.c_str());
            if (s.size() > 0 && s[0]!='#')
                excludes.insert(s);
            }
        fclose(f);
        }

    listFilesInDirectory(".", 0);

    // Note which files in the exclude list were not used.
    std::set<String>::iterator iter;
    for (iter=excludes.begin() ; iter!=excludes.end() ; iter++)
        {
        String fname = *iter;
        std::set<String>::iterator citer = excludesUsed.find(fname);
        if (citer == excludesUsed.end())
            excludesUnused.insert(fname);
        }

    saveFileList();

    return true;
}


/**
 * Get a character from the buffer at pos.  If out of range,
 * return -1 for safety
 */
int DepTool::get(int pos)
{
    if (pos>fileSize)
        return -1;
    return fileBuf[pos];
}



/**
 *  Skip over all whitespace characters beginning at pos.  Return
 *  the position of the first non-whitespace character.
 */
int DepTool::skipwhite(int pos)
{
    while (pos < fileSize)
        {
        int ch = get(pos);
        if (ch < 0)
            break;
        if (!isspace(ch))
            break;
        pos++;
        }
    return pos;
}


/**
 *  Parse the buffer beginning at pos, for a word.  Fill
 *  'ret' with the result.  Return the position after the
 *  word.
 */
int DepTool::getword(int pos, String &ret)
{
    while (pos < fileSize)
        {
        int ch = get(pos);
        if (ch < 0)
            break;
        if (isspace(ch))
            break;
        ret.push_back((char)ch);
        pos++;
        }
    return pos;
}

/**
 * Return whether the sequence of characters in the buffer
 * beginning at pos match the key,  for the length of the key
 */
bool DepTool::sequ(int pos, char *key)
{
    while (*key)
        {
        if (*key != get(pos))
            return false;
        key++; pos++;
        }
    return true;
}



/**
 *  Add an include file name to a file record.  If the name
 *  is not found in allFiles explicitly, try prepending include
 *  directory names to it and try again.
 */
bool DepTool::addIncludeFile(FileRec *frec, const String &iname)
{

    std::map<String, FileRec *>::iterator iter =
           allFiles.find(iname);
    if (iter != allFiles.end())
        {
         //h file in same dir
        FileRec *other = iter->second;
        //trace("local: '%s'", iname.c_str());
        frec->files[iname] = other;
        return true;
        }
    else 
        {
        //look in other dirs
        std::vector<String>::iterator diter;
        for (diter=directories.begin() ;
             diter!=directories.end() ; diter++)
            {
            String dfname = *diter;
            dfname.append("/");
            dfname.append(iname);
            iter = allFiles.find(dfname);
            if (iter != allFiles.end())
                {
                FileRec *other = iter->second;
                //trace("other: '%s'", iname.c_str());
                frec->files[dfname] = other;
                return true;
                }
            }
        }
    return true;
}



/**
 *  Lightly parse a file to find the #include directives.  Do
 *  a bit of state machine stuff to make sure that the directive
 *  is valid.  (Like not in a comment).
 */
bool DepTool::scanFile(const String &fname, FileRec *frec)
{
    FILE *f = fopen(fname.c_str(), "r");
    if (!f)
        {
        error("Could not open '%s' for reading", fname.c_str());
        return false;
        }
    String buf;
    while (!feof(f))
        {
        int len = fread(readBuf, 1, readBufLen, f);
        readBuf[len] = '\0';
        buf.append(readBuf);
        }
    fclose(f);

    fileSize = buf.size();
    fileBuf  = (char *)buf.c_str();
    int pos = 0;


    while (pos < fileSize)
        {
        //trace("p:%c", get(pos));

        //# Block comment
        if (get(pos) == '/' && get(pos+1) == '*')
            {
            pos += 2;
            while (pos < fileSize)
                {
                if (get(pos) == '*' && get(pos+1) == '/')
                    {
                    pos += 2;
                    break;
                    }
                else
                    pos++;
                }
            }
        //# Line comment
        else if (get(pos) == '/' && get(pos+1) == '/')
            {
            pos += 2;
            while (pos < fileSize)
                {
                if (get(pos) == '\n')
                    {
                    pos++;
                    break;
                    }
                else
                    pos++;
                }
            }
        //# #include! yaay
        else if (sequ(pos, "#include"))
            {
            pos += 8;
            pos = skipwhite(pos);
            String iname;
            pos = getword(pos, iname);
            if (iname.size()>2)
                {
                iname = iname.substr(1, iname.size()-2);
                addIncludeFile(frec, iname);
                }
            }
        else
            {
            pos++;
            }
        }

    return true;
}



/**
 *  Recursively check include lists to find all files in allFiles to which
 *  a given file is dependent.
 */
bool DepTool::processDependency(FileRec *ofile,
                                FileRec *include,
                                int depth)
{
    std::map<String, FileRec *>::iterator iter;
    for (iter=include->files.begin() ; iter!=include->files.end() ; iter++)
        {
        String fname  = iter->first;
        if (ofile->files.find(fname) != ofile->files.end())
            {
            //trace("file '%s' already seen", fname.c_str());
            continue;
            }
        FileRec *child  = iter->second;
        ofile->files[fname] = child;
      
        processDependency(ofile, child, depth+1);
        }


    return true;
}



/**
 *  Recursively check include lists to find all files in allFiles which
 *  will eventually have a dependency on this file.  This is basically
 *  the inverse of processDependency().
 */
bool DepTool::processReference(FileRec *hfile,
                               String &fname,
                               int depth)
{
    std::map<String, FileRec *>::iterator iter;
    for (iter=allFiles.begin() ; iter!=allFiles.end() ; iter++)
        {
        FileRec *frec = iter->second;
        std::map<String, FileRec *>::iterator fiter =
                  frec->files.find(fname);
        if (fiter != frec->files.end())
            {
            String cfname  = iter->first;
            if (hfile->files.find(cfname) != hfile->files.end())
                {
                //trace("%d reffile '%s' already seen", depth, cfname.c_str());
                continue;
                }
            FileRec *child  = iter->second;
            child->distance = depth;
            hfile->files[cfname] = child;      
            processReference(hfile, cfname, depth+1);
            }
        }

    return true;
}



/**
 *  Generate the file dependency and reference lists.
 */
bool DepTool::generateDependencies()
{
    std::map<String, FileRec *>::iterator iter;
    //# First pass.  Scan for all includes
    for (iter=allFiles.begin() ; iter!=allFiles.end() ; iter++)
        {
        FileRec *frec = iter->second;
        if (!scanFile(iter->first, frec))
            {
            //quit?
            }
        }

    //# Second pass.  Scan for all includes
    for (iter=allFiles.begin() ; iter!=allFiles.end() ; iter++)
        {
        FileRec *include = iter->second;
        if (include->type == FileRec::CFILE)
            {
            String cFileName = iter->first;
            FileRec *ofile     = new FileRec(FileRec::OFILE);
            ofile->path        = include->path;
            ofile->baseName    = include->baseName;
            ofile->suffix      = include->suffix;
            String fname     = include->path;
            if (fname.size()>0)
                fname.append("/");
            fname.append(include->baseName);
            fname.append(".o");
            depFiles[fname]    = ofile;
            //add the .c file first, of course
            ofile->files[cFileName] = include;
            //trace("ofile:%s", fname.c_str());

            processDependency(ofile, include, 0);
            }
        /*
        else if (include->type == FileRec::HFILE)
            {
            String fname     = iter->first;
            FileRec *hfile     = new FileRec(FileRec::HFILE);
            hfile->path        = include->path;
            hfile->baseName    = include->baseName;
            hfile->suffix      = include->suffix;
            refFiles[fname]    = hfile;
            //trace("hfile:%s", fname.c_str());

            processReference(hfile, fname, 0);
            }
        */
        }

      
    return true;
}


/**
 *  High-level call to do what DepTool does.
 */
bool DepTool::run()
{
    reset();
    if (!createFileList())
        return false;
    if (!generateDependencies())
        return false;
    saveDepFile(false);
    saveCmakeFile();
    //saveRefFile(true);
    return true;
}


//########################################################################
//# O U T P U T S
//########################################################################



/**
 *  Save the allFiles list.  This is basically all files in a directory
 *  except those denied in the exclude list.
 */
bool DepTool::saveFileList()
{
    time_t tim;
    time(&tim);

    FILE *f = fopen("make.files", "w");
    if (!f)
        {
        trace("cannot open 'make.files' for writing");
        }
    fprintf(f, "########################################################\n");
    fprintf(f, "## File: make.files\n");
    fprintf(f, "## Generated by DepTool at :%s", ctime(&tim));
    fprintf(f, "########################################################\n");

    fprintf(f, "\n\n\n");
    fprintf(f, "########################################################\n");
    fprintf(f, "## F I L E S\n");
    fprintf(f, "########################################################\n");

    std::map<String, FileRec *>::iterator iter;
    for (iter=allFiles.begin() ; iter!=allFiles.end() ; iter++)
        {
        fprintf(f, "%s\n", iter->first.c_str());
        }

    fprintf(f, "\n\n\n");
    fprintf(f, "########################################################\n");
    fprintf(f, "## E X C L U D E D\n");
    fprintf(f, "########################################################\n");

    std::set<String>::iterator uiter;
    for (uiter=excludesUsed.begin() ; uiter!=excludesUsed.end() ; uiter++)
        {
        String fname = *uiter;
        fprintf(f, "%s\n", fname.c_str());
        }

    fprintf(f, "\n\n\n");
    fprintf(f, "########################################################\n");
    fprintf(f, "## E X C L U D E  entries unused\n");
    fprintf(f, "########################################################\n");

    for (uiter=excludesUnused.begin() ; uiter!=excludesUnused.end() ; uiter++)
        {
        String fname = *uiter;
        fprintf(f, "%s\n", fname.c_str());
        }

    fprintf(f, "\n\n\n");
    fprintf(f, "########################################################\n");
    fprintf(f, "## E N D\n");
    fprintf(f, "########################################################\n");

    fclose(f);

    return true;
}


/**
 *   This is the main product.  This file lists the Include directives,
 *   the Object list, and the dependency list.
 */
bool DepTool::saveDepFile(bool doXml)
{
    time_t tim;
    time(&tim);

    FILE *f = fopen("make.dep", "w");
    if (!f)
        {
        trace("cannot open 'make.dep' for writing");
        }
    if (doXml)
        {
        fprintf(f, "<?xml version='1.0'?>\n");
        fprintf(f, "<deptool>\n");
        fprintf(f, "\n");
        fprintf(f, "<!--\n");
        fprintf(f, "########################################################\n");
        fprintf(f, "## File: make.dep\n");
        fprintf(f, "## Generated by DepTool at :%s", ctime(&tim));
        fprintf(f, "########################################################\n");
        fprintf(f, "-->\n");

        fprintf(f, "\n\n");
    
        fprintf(f, "\n");
        fprintf(f, "<!--\n");
        fprintf(f, "########################################################\n");
        fprintf(f, "## I N C L U D E\n");
        fprintf(f, "########################################################\n");
        fprintf(f, "-->\n");
        fprintf(f, "<includes>\n");
    
        std::vector<String>::iterator inciter;
        for (inciter=directories.begin() ; inciter!=directories.end() ; inciter++)
            {
            String dirname = *inciter;
            fprintf(f, "    <inc name='%s'/>\n", dirname.c_str());
            }

        fprintf(f, "</includes>\n");
        fprintf(f, "\n\n\n");
        fprintf(f, "<!--\n");
        fprintf(f, "########################################################\n");
        fprintf(f, "## O B J E C T S\n");
        fprintf(f, "########################################################\n");
        fprintf(f, "-->\n");
        fprintf(f, "<objects>\n");
    
        std::map<String, FileRec *>::iterator oiter;
        for (oiter=allFiles.begin() ; oiter!=allFiles.end() ; oiter++)
            {
            FileRec *frec = oiter->second;
            if (frec->type == FileRec::CFILE)
                {
                String fname = frec->path;
                if (fname.size()>0)
                    fname.append("/");
                fname.append(frec->baseName);
                fname.append(".o");
                fprintf(f, "   <obj name='%s'/>\n", fname.c_str());
                }
            }


        fprintf(f, "</objects>\n");
        fprintf(f, "\n\n\n");
        fprintf(f, "<!--\n");
        fprintf(f, "########################################################\n");
        fprintf(f, "## D E P E N D E N C I E S\n");
        fprintf(f, "########################################################\n");
        fprintf(f, "-->\n");
        fprintf(f, "<dependencies>\n\n");
        std::map<String, FileRec *>::iterator iter;
        for (iter=depFiles.begin() ; iter!=depFiles.end() ; iter++)
            {
            FileRec *frec = iter->second;
            if (frec->type == FileRec::OFILE)
                {
                String fname = iter->first;
                fprintf(f, "<file name='%s'>\n", fname.c_str());
                std::map<String, FileRec *>::iterator citer;
                for (citer=frec->files.begin() ; citer!=frec->files.end() ; citer++)
                    {
                    String cfname = citer->first;
                    fprintf(f, "    <dep name='%s'/>\n", cfname.c_str());
                    }
                fprintf(f, "</file>\n\n");
                }
            }

        fprintf(f, "</dependencies>\n");
        fprintf(f, "\n\n\n");
        fprintf(f, "</deptool>\n");
        fprintf(f, "<!--\n");
        fprintf(f, "########################################################\n");
        fprintf(f, "## E N D\n");
        fprintf(f, "########################################################\n");
        fprintf(f, "-->\n");
        }
    else // ######### !XML
        {
        fprintf(f, "########################################################\n");
        fprintf(f, "## File: make.dep\n");
        fprintf(f, "## Generated by DepTool at :%s", ctime(&tim));
        fprintf(f, "########################################################\n");

        fprintf(f, "\n\n");
    
        fprintf(f, "########################################################\n");
        fprintf(f, "## I N C L U D E\n");
        fprintf(f, "########################################################\n");
        fprintf(f, "DEPTOOL_INCLUDE =");
    
        std::vector<String>::iterator inciter;
        for (inciter=directories.begin() ; inciter!=directories.end() ; inciter++)
            {
            fprintf(f, " \\\n");
            String dirname = *inciter;
            fprintf(f, "-I%s", dirname.c_str());
            }

        fprintf(f, "\n\n\n");
        fprintf(f, "########################################################\n");
        fprintf(f, "## O B J E C T S\n");
        fprintf(f, "########################################################\n");
        fprintf(f, "DEPTOOL_OBJECTS =");
    
        std::map<String, FileRec *>::iterator oiter;
        for (oiter=allFiles.begin() ; oiter!=allFiles.end() ; oiter++)
            {
            FileRec *frec = oiter->second;
            if (frec->type == FileRec::CFILE)
                {
                fprintf(f, " \\\n");
                String fname = frec->path;
                if (fname.size()>0)
                    fname.append("/");
                fname.append(frec->baseName);
                fname.append(".o");
                fprintf(f, "%s", fname.c_str());
                }
            }


        fprintf(f, "\n\n\n");
        fprintf(f, "########################################################\n");
        fprintf(f, "## D E P E N D E N C I E S\n");
        fprintf(f, "########################################################\n");
        std::map<String, FileRec *>::iterator iter;
        for (iter=depFiles.begin() ; iter!=depFiles.end() ; iter++)
            {
            FileRec *frec = iter->second;
            if (frec->type == FileRec::OFILE)
                {
                String fname = iter->first;
                fprintf(f, "%s:", fname.c_str());
                std::map<String, FileRec *>::iterator citer;
                for (citer=frec->files.begin() ; citer!=frec->files.end() ; citer++)
                    {
                    String cfname = citer->first;
                    fprintf(f, " \\\n");
                    fprintf(f, "\t%s", cfname.c_str());
                    }
                fprintf(f, "\n\n\n");
                }
            }

        fprintf(f, "\n\n\n");
        fprintf(f, "########################################################\n");
        fprintf(f, "## E N D\n");
        fprintf(f, "########################################################\n");
        }

    fclose(f);

    return true;
}


/**
 *  Save the "reference" file, which lists each include file, and any files
 *  that are judged to be dependent upon it.
 */
bool DepTool::saveRefFile(bool doXml)
{
    time_t tim;
    time(&tim);

    FILE *f = fopen("make.ref", "w");
    if (!f)
        {
        trace("cannot open 'make.ref' for writing");
        }
    if (doXml)
        {
        fprintf(f, "<?xml version='1.0'?>\n");
        fprintf(f, "<deptool>\n\n");
        fprintf(f, "<!--\n");
        fprintf(f, "########################################################\n");
        fprintf(f, "## File: make.ref\n");
        fprintf(f, "## Generated by DepTool at :%s", ctime(&tim));
        fprintf(f, "## Note: The metric is the 'distance' of inclusion from\n");
        fprintf(f, "##    the given file.\n");
        fprintf(f, "########################################################\n");
        fprintf(f, "-->\n");
        fprintf(f, "\n\n");
    

        std::map<String, FileRec *>::iterator iter;
        for (iter=refFiles.begin() ; iter!=refFiles.end() ; iter++)
            {
            FileRec *frec = iter->second;
            if (frec->type == FileRec::HFILE)
                {
                String fname = iter->first;
                fprintf(f, "<file name='%s'>\n", fname.c_str());
                std::map<String, FileRec *>::iterator citer;
                for (citer=frec->files.begin() ; citer!=frec->files.end() ; citer++)
                    {
                    String cfname = citer->first;
                    fprintf(f, "    <ref d='%d' name='%s'/>\n", 
                               citer->second->distance, cfname.c_str());
                    }
                fprintf(f, "</file>\n\n");
                }
            }
        fprintf(f, "\n\n\n");
        fprintf(f, "</deptool>\n");
        fprintf(f, "<!--\n");
        fprintf(f, "########################################################\n");
        fprintf(f, "## E N D\n");
        fprintf(f, "########################################################\n");
        fprintf(f, "-->\n");
        }

    else //######### not xml
        {
        fprintf(f, "########################################################\n");
        fprintf(f, "## File: make.ref\n");
        fprintf(f, "## Generated by DepTool at :%s", ctime(&tim));
        fprintf(f, "## Note: The metric is the 'distance' of inclusion from\n");
        fprintf(f, "##    the given file.\n");
        fprintf(f, "########################################################\n");
        fprintf(f, "\n\n");
    

        std::map<String, FileRec *>::iterator iter;
        for (iter=refFiles.begin() ; iter!=refFiles.end() ; iter++)
            {
            FileRec *frec = iter->second;
            if (frec->type == FileRec::HFILE)
                {
                String fname = iter->first;
                fprintf(f, "### %s\n", fname.c_str());
                std::map<String, FileRec *>::iterator citer;
                for (citer=frec->files.begin() ; citer!=frec->files.end() ; citer++)
                    {
                    String cfname = citer->first;
                    fprintf(f, "%3d %s\n", citer->second->distance,
                                   cfname.c_str());
                    }
                fprintf(f, "\n");
                }
            }

        fprintf(f, "\n\n\n");
        fprintf(f, "########################################################\n");
        fprintf(f, "## E N D\n");
        fprintf(f, "########################################################\n");
        }

    fclose(f);

    return true;
}


/**
 *   This is a new thing.  It creates a cmake file that should be able to
 *   build the entire thing. 
 */
bool DepTool::saveCmakeFile()
{
    time_t tim;
    time(&tim);

    FILE *f = fopen("CMakeLists.txt", "w");
    if (!f)
        {
        trace("cannot open 'CMakeLists.txt' for writing");
        }
    fprintf(f, "########################################################\n");
    fprintf(f, "## File: CMakeLists.txt\n");
    fprintf(f, "## Generated by DepTool at :%s", ctime(&tim));
    fprintf(f, "########################################################\n");

    fprintf(f, "\n\n");
    
    fprintf(f, "\n\n\n");
    fprintf(f, "########################################################\n");
    fprintf(f, "## P R O J E C T\n");
    fprintf(f, "########################################################\n");
    fprintf(f, "project (INKSCAPE)\n");
    fprintf(f, "\n\n\n");
    fprintf(f, "########################################################\n");
    fprintf(f, "## O B J E C T S\n");
    fprintf(f, "########################################################\n");
    fprintf(f, "set (INKSCAPE_SRCS\n");
    
    std::map<String, FileRec *>::iterator oiter;
    for (oiter=allFiles.begin() ; oiter!=allFiles.end() ; oiter++)
        {
        FileRec *frec = oiter->second;
        if (frec->type == FileRec::CFILE)
            {
            //fprintf(f, " \\\n");
            String fname = frec->path;
            if (fname.size()>0)
                fname.append("/");
            fname.append(frec->baseName);
            fname.append(".");
            fname.append(frec->suffix);
            fprintf(f, "%s\n", fname.c_str());
            }
        }
    fprintf(f, ")\n\n");

    fprintf(f, "add_executable (inkscape ${INKSCAPE_SRCS})\n");

    fprintf(f, "\n\n\n");
    fprintf(f, "########################################################\n");
    fprintf(f, "## E N D\n");
    fprintf(f, "########################################################\n");

    fclose(f);

    return true;
}






//########################################################################
//# M A I N
//########################################################################


/**
 *  Run the DepTool's main functions
 */
static bool runTool()
{
    DepTool depTool;

    if (!depTool.run())
        return false;

    return true;
}


/**
 * Console main()
 */
int main(int argc, char **argv)
{
    if (!runTool())
        return 1;

    return 0;
}


//########################################################################
//# E N D    O F    F I L E
//########################################################################
