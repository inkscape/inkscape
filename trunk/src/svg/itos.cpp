/////////////////////////////////////////////////////////////////////////
//                               ftoa.cpp
//
// Copyright (c) 1996-2003 Bryce W. Harrington  [bryce at osdl dot org]
//
//-----------------------------------------------------------------------
// License:  This code may be used by anyone for any purpose
//           so long as the copyright notices and this license
//           statement remains attached.
//-----------------------------------------------------------------------
//
// This routine converts an integer into a string
//
/////////////////////////////////////////////////////////////////////////

// Standard include files
#include <algorithm>
#include <string>		// for string
#include <cstring>

using namespace std;

string itos(int n)
{
  int sign;
  string s;

  if ((sign = n) < 0)           // record sign
    n = -n;                     // make n positive
  do {                          // generate digits in reverse order
    s += (char(n % 10) + '0');   // get next digit
  } while ((n/=10) > 0);        // delete it

  if (sign < 0)
    s += '-';

  reverse(s.begin(), s.end());  // This is what the code should look like
                                // if the string class is compatible with
                                // the standard C++ string class
#ifdef DUMB_OS_LIKE_WINDOWS
  // In Windows, we'll use this hack...
  for (int i=0, j=s.GetLength()-1; i<j; i++, j--)
  {
	  char c = s[i];
//	  s[i] = s[j];
//	  s[j] = c;
      s.SetAt(i, s[j]);
      s.SetAt(j, c);
  }
#endif

  return s;
}

string ultos(unsigned long n)
{
  string s;

  do {                          // generate digits in reverse order
    s += (char(n % 10) + '0');   // get next digit
  } while ((n/=10) > 0);        // delete it

  reverse(s.begin(), s.end());  // This is what the code should look like
                                // if the string class is compatible with
                                // the standard C++ string class
#ifdef DUMB_OS_LIKE_WINDOWS
  // In Windows, we'll use this hack...
  for (int i=0, j=s.GetLength()-1; i<j; i++, j--)
  {
	  char c = s[i];
//	  s[i] = s[j];
//	  s[j] = c;
      s.SetAt(i, s[j]);
      s.SetAt(j, c);
  }
#endif

  return s;
}
