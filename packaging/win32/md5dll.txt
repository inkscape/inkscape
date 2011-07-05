Calculates the md5sum of a file or string.
Has been tested to work with NSIS 2.0+


Derived from the RSA Data Security, Inc. MD5 Message-Digest Algorithm

[Md5Dll]
Matthew "IGx89" Lieder
  -Original plugin Author

Sunjammer (12th May 2002)
  -Original usage notes and example script 

[Md5Dll.0.1]
KJD (2004)
  -Modified to reduce size and use exdll.h
   (reduced to about 6KB uncompressed, by removing CRTL dependency)

[Md5dll.0.2]
Davy Durham (2004)
  -MD5.cpp fix (correct for loop used to replace memset, exceeded bounds)

[Md5dll.0.3]
Shengalts Aleksander aka Instructor (2005)
  -New command: "GetMD5Random"
  -Changed names: "GetFileMD5" -> "GetMD5File", "GetMD5" -> "GetMD5String"
  -Fixed: string length error

[Md5dll.0.4]
KJD (2005)
  -Added dual name to exports for backwards compatibility


--------------------------------

Usage:

Push $1 ;string
CallInstDll "md5dll" GetMD5String
Pop $1 ;md5 of string

-or-

Push $1 ;filename
CallInstDll "md5dll" GetMD5File
Pop $1 ;md5 of file

--------------------------------

Example usage in recent NSIS versions

OutFile "md5test.exe"
Section ""
  #generate MD5sum of a string
  md5dll::GetMD5String "md5me"
  Pop $0
  DetailPrint "md5: [$0]"

  # generate MD5sum of a file
  md5dll::GetMD5File "${NSISDIR}\makensis.exe"
  Pop $0
  DetailPrint "md5: [$0]"

  #generate random MD5sum
  md5dll::GetMD5Random
  Pop $0
  DetailPrint "md5: [$0]"
SectionEnd
