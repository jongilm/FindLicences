// FindLicences.cpp : Defines the entry point for the console application.
//
#define VERBOSE0
//#define VERBOSE1
//#define VERBOSE2
//#define DUMMY_RUN
//#define TEST_WALK

#pragma warning( disable : 4996 ) // _CRT_SECURE_NO_WARNINGS
//#define DEPRECATE_SUPPORTED

#include "stdafx.h"
#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <io.h>
#include <fcntl.h>
#include <string>

typedef int BOOL;
#define TRUE 1
#define FALSE 0

#define ERC_STRING_FOUND_IN_FILE     (2)
#define ERC_STRING_NOT_FOUND_IN_FILE (1)
#define ERC_OK                       (0)
#define ERC_INVALID_ARGS             (-1)
#define ERC_CANNOT_OPEN_INPUT_FILE   (-2)
#define ERC_UNEXPECTED_EOF           (-3)
#define ERC_CANNOT_OPEN_OUTPUT_FILE  (-4)
#define ERC_FILESIZE_ERROR           (-5)
#define ERC_IS_BINARY_FILE           (-6)

static char szErrorString[_MAX_PATH + 80] = {0};

static char *szSearchStrings[] = 
{
   "copyright",
   "licen",
   "GPL",
   "Written by",
   "Author",
   "public domain",
   "grant",
   "prohibit",
   "authori",
   "permit",
   "permis",
   "(C)",
   "Wind River",
   "WindManage",
   "RSA Data Security",
   "Cavium",
   "Motorola",
   "Mesh",
   "MeshNetworks",
   "Intoto",
   "Symbol Technologies",
   "Green Hills",
   "Mocana",
   "Hewlett-Packard",
   "Hewlett",
   "Express Logic",
   "Free Software Foundation",
   "Free Software",
   "Broadcom",
   "Intel Corporation",
   "SysKonnect",
   "Sysgo",
   "Linus Torvalds",
   "Torvalds",
   "Altera",
   "Red Hat",
   "Infineon",
   "Quadralay",
   "Carnegie Mellon",
   "Atheros",
   "Brian Gladman",
   "Massachusetts Institute of Technology",
   "FundsXpress",
   "Intersil",
   "Sam Leffler",
   "Errno Consulting",
   "Epilogue Technology",
   "Integrated Systems",
   "SSH Communications",
   "Timo J. Rinne",
   "Rapid Logic",
   "Eric Young",
   "cryptsoft",
   "OpenSSL Project",
   "Anders Hedstrom",
   
   // Not in original list
   "Academic Free License",
   "Affero General Public License",
   "Affero GPL",
   "AFL",
   "Apache License",
   "Apple Public Source License",
   "APSL",
   "Artistic License",
   "BDL",
   "Berkeley Database License",
   "Boost Software License",
   "BSD Documentation License",
   "BSD license",
   "BSD",
   "CDDL",
   "CEA CNRS INRIA Logiciel Libre",
   "CeCILL",
   "commercial use",
   "Common Development and Distribution License",
   "Common Public License",
   "Cryptix General License",
   "Debian",
   "Eclipse Public License",
   "Educational Community License",
   "Eiffel Forum License",
   "EPL",
   "EULA",
   "EUPL",
   "European Union Public License",
   "Free For non commercial Use",
 //"Free Software",
   "Free Solaris Binary License",
   "GNU General Public License",
   "GNU Lesser General Public License",
 //"GPL",
   "Hacktivismo Enhanced-Source Software License Agreement",
   "IBM Public License",
   "Intel Open Source License",
   "ISC license",
   "LaTeX Project Public License",
   "LGPL",
   "libpng license",
   "License of Perl",
   "License of Python",
   "LPPL",
   "Microsoft Public License",
   "Microsoft Reciprocal License",
   "Microsoft Reference License",
   "Microsoft Shared Sources",
   "Microsoft Windows EULA",
   "Microsoft",
   "MIT license",
   "Mozilla Public License",
   "MPL",
   "MS-RL",
   "Netscape Public License",
   "NPL",
   "OPaC Free Public License",
   "Open Software License",
   "Open Source Definition",
   "Open Source Initiative",
   "Open Source",
   "OpenSource",
   "OpenSSL license",
   "OSD",
   "OSI",
   "PHP License",
   "Poetic License",
 //"Public Domain",
   "Python Software Foundation License",
   "Q Public License",
   "QPL",
   "SISSL",
   "Sleepycat License",
   "Sleepycat Software Product License",
   "Sun Industry Standards Source License",
   "Sun Public License",
   "Sybase Open Watcom Public License",
   "W3C Software Notice and License",
   "WTFPL",
   "X11 license",
   "XFree86",
   "zlib license",
   "Zope Public License"
};

#if !defined(TEST_WALK)
static int IsDelimiter(char ch)
{
   if (ch == 0 || isspace(ch) || iscntrl(ch) || !isalnum(ch) )
      return TRUE;
   return FALSE;
}
#endif

#if defined(TEST_WALK)
static long GetFileSize(char *filename)
{
   int handle;
   long length = 0;

   handle = open( filename, O_BINARY | O_RDONLY );
   length = filelength( handle );
   close( handle );

   return(length);
}
#endif

static int IsPrintableChar(unsigned char ch)
{
   if (ch > 0x7F)
      return FALSE;
  
   if (isprint(ch) || 
       ch == '\n' || 
       ch == '\r' || 
       ch == '\t')
   {
      return TRUE;
   }
   return FALSE;
}

static int IsBinaryFile(char *argFilename)
{
   FILE *fInputFile;
   unsigned char buf[4];
   int i;

   fInputFile = fopen(argFilename,"rb");
   if (fInputFile)
   {
     for (i=0;i<128;i++)
     {
        if (fread(buf,1,1,fInputFile) == 1) 
        {
           if (!IsPrintableChar(buf[0]))
           {
              fclose(fInputFile);
              return TRUE;
           }
        }
        else
          break;
      }
      fclose(fInputFile);
   }
   return FALSE;
}

#if !defined(TEST_WALK)
static int FindString(char *buffer, char *szSearchString)
{
   char *pos1;
   char  LeadingChar;
   char  TrailingChar;
   int   SearchStringLen;

   SearchStringLen = strlen(szSearchString);

   // Find the string
   pos1 = strstr(buffer,szSearchString);
   if (pos1)
   {
      // String is found, but ensure that it is not a part
      // of some larger word eg GPL and ShoppingPlace
      // i.e. check that the chars on each side are valid delimeters.
      TrailingChar = *(pos1 + SearchStringLen);
      if (pos1 == buffer) // Found at beginning of string
         LeadingChar = 0;
      else // Found, but not at beginning of string
         LeadingChar = *(pos1-1);

      // If the char on either side is alphanumeric, then this is not a hit.
      if (IsDelimiter(LeadingChar) && IsDelimiter(TrailingChar))
         return TRUE;
   }
   return FALSE;
}
#endif // !defined(TEST_WALK)

int ReplaceSubString(char *szSearchString,char *OldSubstring,char *NewSubstring)
{
   int count = 0;
   char *pos;

   if (strlen(OldSubstring) != strlen(NewSubstring))
      return 0;

   for(;;)
   {
      if (strlen(szSearchString)==0)
         break;
      pos = strstr(szSearchString,OldSubstring);
      if (pos == NULL)
         break;
      memcpy(pos,NewSubstring,strlen(NewSubstring));
      count++;
      szSearchString = pos+strlen(NewSubstring);
   }
   return count;
}
/*
static char *my_fgets(char *s, int n, FILE *f)
{
  int c = 0;
  char *cs;

  cs = s;
  while (--n>0 && (c = getc(f)) != EOF)
  {
    *cs++ = c;
    if (c == '\n')
      break;
  }
  if (c == EOF && cs == s)
    return NULL;
  *cs++ = '\0';
  return s;
}
*/
static int SearchFileForString(char *argFilename, char *argSearchString)
{
   FILE *fInputFile;
   int   rc = ERC_STRING_NOT_FOUND_IN_FILE;
   char buffer[4096];
   char *ret;
   char szSearchStringCopy[128];
#if defined(TEST_WALK)
   long ActualFileSize;
   long CalculatedFileSize = 0;

   ActualFileSize = GetFileSize(argFilename);
#else
   char szSearchStringAlternate[128];
#endif

   strcpy(szSearchStringCopy,argSearchString);

   // Check if this is a unicode file - not supported at this time
   if (IsBinaryFile(argFilename))
   {
     return ERC_IS_BINARY_FILE;
   }

   fInputFile = fopen(argFilename,"rb");
   if (!fInputFile)
   {
      sprintf(szErrorString,"Cannot open input file: \"%s\"",argFilename);
      return ERC_CANNOT_OPEN_INPUT_FILE;
   }

   // Convert both to lowercase to make the search case-insensitive
   strlwr(szSearchStringCopy);
   
   for (;;)
   {
      ret = fgets((char *)buffer,sizeof(buffer),fInputFile);
      if (ret == NULL)
      {
         if (feof(fInputFile))
         {
            rc = ERC_STRING_NOT_FOUND_IN_FILE;
         }
         else
         {
            sprintf(szErrorString,"Unexpected EOF: \"%s\"",argFilename);
            rc = ERC_UNEXPECTED_EOF;
         }
         break;
      }
#if !defined(TEST_WALK)
      if (strlen(buffer) == 0)
         continue;

      // Convert both to lowercase to make the search case-insensitive
      strlwr(buffer);

      // Search for original string
      if (FindString(buffer,szSearchStringCopy) == TRUE)
      {
         rc = ERC_STRING_FOUND_IN_FILE;
         break;
      }

      // Search again with alternate spelling
      strcpy(szSearchStringAlternate,szSearchStringCopy);
      if (ReplaceSubString(szSearchStringAlternate,"licence","license") > 0)
      {
         if (FindString(buffer,szSearchStringAlternate) == TRUE)
         {
            rc = ERC_STRING_FOUND_IN_FILE;
            break;
         }
      }
      
      // Search again with alternate spelling
      strcpy(szSearchStringAlternate,szSearchStringCopy);
      if (ReplaceSubString(szSearchStringAlternate,"license","licence") > 0)
      {
         if (FindString(buffer,szSearchStringAlternate) == TRUE)
         {
            rc = ERC_STRING_FOUND_IN_FILE;
            break;
         }
      }
      // Search again with hyphens replaced by spaces
      strcpy(szSearchStringAlternate,szSearchStringCopy);
      if (ReplaceSubString(szSearchStringAlternate,"-"," ") > 0)
      {
         if (FindString(buffer,szSearchStringAlternate) == TRUE)
         {
            rc = ERC_STRING_FOUND_IN_FILE;
            break;
         }
      }
#else  // !defined(TEST_WALK)
      CalculatedFileSize += strlen((char *)buffer);
#endif // !defined(TEST_WALK)
      if (feof(fInputFile))
      {
         rc = ERC_STRING_NOT_FOUND_IN_FILE;
         break;
      }
   }

   fclose(fInputFile);
#if defined(TEST_WALK)
   if (CalculatedFileSize != ActualFileSize)
   {
      sprintf(szErrorString,"File size mismatch: \"%s\" [Calculated=%ld, Actual=%ld]",argFilename,CalculatedFileSize, ActualFileSize);
      rc = ERC_FILESIZE_ERROR;
   }
   else
   {
     // Pretend the string was found so that it will be written to output file
     rc = ERC_STRING_FOUND_IN_FILE;
   }
#endif
   return rc;
}

static int ProcessFile(char *szFilename, char *szSearchString, FILE *fOutFile)
{
   int   rc;

#ifdef VERBOSE2
   printf("Processing \"%s\"\n",szFilename); 
#endif

   rc = SearchFileForString(szFilename,szSearchString);
   if (rc == ERC_STRING_FOUND_IN_FILE)
   {
#ifdef VERBOSE1
      printf("   BINGO: \"%s\" found in \"%s\"\n",szSearchString,szFilename); 
#endif
#if defined(DUMMY_RUN)
      (void)fOutFile;
#else
      fprintf(fOutFile,"%s\r\n",szFilename);
#endif
      // All is well, so replace 1 (meaing string was found)
      // with a 0 (all is well)
      rc = ERC_OK;
   }
   else if (rc == ERC_IS_BINARY_FILE)
   {
#if !defined(DUMMY_RUN)
      fprintf(fOutFile,"%s <=== BINARY FILE. SKIPPED.\r\n",szFilename);
#endif
      rc = ERC_OK;
   }
   else if (rc == ERC_STRING_NOT_FOUND_IN_FILE)
   {
      rc = ERC_OK;
   }


   return rc;
}

/***************************************************************************
 * Function: dir_scan
 *
 * Purpose:
 *
 * Scan the directory given. Add all files to the list
 * in alphabetic order, and add all directories in alphabetic
 * order to the list of child DIRITEMs. If bRecurse is true, go on to
 * recursive call dir_scan for each of the child DIRITEMs
 */
static int dir_scan(char *szStartDir, BOOL bRecurse, char *szSearchString, FILE *fOutFile)
{
   char szPath[1024];
   BOOL bMore;
   BOOL bIsDir;
   HANDLE hFind;
   WIN32_FIND_DATA finddata;
   int rc = ERC_OK;
   char szFullFilename[_MAX_PATH];
   TCHAR wcsPath[_MAX_PATH];

#ifdef VERBOSE2
   printf("Scanning: \"%s\"\n",szStartDir);
#endif

   // Make the complete search string including *.*
   strcpy(szPath, szStartDir);
 
   // Omit the . at the beginning of the relname, and the
   // .\ if there is a trailing \ on the rootname
   //if (*CharPrev(szPath, szPath+strlen(szPath)) != '\\')  // CharPrev(pStart, pCurrent)

   if (strlen(szPath)>0 && szPath[strlen(szPath)-1] != '\\')
      strcat(szPath, "\\");
   strcat(szPath, "*.*");

   // Convert szPath to a WideCharacter string
   //errno_t mbstowcs_s(
   //   size_t *pReturnValue,
   //   wchar_t *wcstr,
   //   size_t sizeInWords,
   //   const char *mbstr,
   //   size_t count 
   //);
   size_t converted_chars = 0;
   mbstowcs_s(&converted_chars,wcsPath,strlen(szPath)+1,szPath,_TRUNCATE);

   // Read all entries in the directory
   hFind = FindFirstFile(wcsPath, &finddata);
   bMore = (hFind != (HANDLE) -1);

   while (bMore) 
   {
      char   szFilename[_MAX_PATH];
      size_t converted_chars = 0;

      // Convert finddata.cFileName to a normal char string
      //errno_t wcstombs_s(
      //   size_t *pReturnValue,
      //   char *mbstr,
      //   size_t sizeInBytes,
      //   const wchar_t *wcstr,
      //   size_t count 
      //);
      wcstombs_s(&converted_chars,szFilename,wcslen(finddata.cFileName)+1,finddata.cFileName,_TRUNCATE);

      strcpy(szFullFilename, szStartDir);
      strcat(szFullFilename, "\\");
      strcat(szFullFilename, szFilename);

      bIsDir = (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
      if (!bIsDir) 
      {
         rc = ProcessFile(szFullFilename, szSearchString, fOutFile);
         if (rc < ERC_OK)
         {
            FindClose(hFind);
            return rc;
         }
      }
      else if ( (strcmp(szFilename, "." ) != 0) && 
                (strcmp(szFilename, "..") != 0) ) 
      {
         if (bRecurse) 
         {
            rc = dir_scan(szFullFilename, TRUE, szSearchString, fOutFile);
            if (rc < ERC_OK)
            {
               FindClose(hFind);
               return rc;
            }
         }
      }
      bMore = FindNextFile(hFind, &finddata);
   }
   FindClose(hFind);

   return rc;
} // dir_scan

static int CatalogFilesContainingString(char *szPath, char *szSearchString)
{
   char szOutputfilename[_MAX_PATH];
   FILE *fOutFile = NULL;
   int rc = ERC_OK;

   sprintf(szOutputfilename,"%s\\FindLicences_%s.txt",szPath,szSearchString);

#ifdef VERBOSE0
   //printf("Search \"%s\" for \"%s\"\n",szPath,szSearchString);
   printf("Creating %s...\n",szOutputfilename);
#endif
#if !defined(DUMMY_RUN)
   fOutFile = fopen(szOutputfilename, "wb");
   if (!fOutFile)
   {
      sprintf(szErrorString,"Error opening output file: \"%s\"",szOutputfilename);
      rc = ERC_CANNOT_OPEN_OUTPUT_FILE;
   }
   else
#endif
   {
      rc = dir_scan(szPath, TRUE, szSearchString, fOutFile);
   }
#if !defined(DUMMY_RUN)
   fclose(fOutFile);
#endif
   return rc;
}

int _tmain(int argc, _TCHAR* argv[])
{
   int rc = ERC_OK;
   char  szPath[_MAX_PATH];
   size_t converted_chars = 0;

   if (argc != 2)
   {
      rc = ERC_INVALID_ARGS;
      strcpy(szErrorString,"Usage: FindLicences <RootDir>");
   }
   else
   {
      // Convert pFilename to a normal char string
      //errno_t wcstombs_s(
      //   size_t *pReturnValue,
      //   char *mbstr,
      //   size_t sizeInBytes,
      //   const wchar_t *wcstr,
      //   size_t count 
      //);
      wcstombs_s(&converted_chars,szPath,wcslen(argv[1])+1,argv[1],_TRUNCATE);

#ifdef TEST_WALK
      rc = CatalogFilesContainingString(szPath, "TEST_WALK");
#else
      for (int i=0; i<sizeof(szSearchStrings)/sizeof(szSearchStrings[0]);i++)
      {
         rc = CatalogFilesContainingString(szPath, szSearchStrings[i]);
         if (rc != ERC_OK)
           break;
      }
#endif
   }

   if (rc != ERC_OK)
   {
     printf("Terminated with error %d: %s\n",rc,szErrorString);
   }
   printf("Press any key to continue\n");
   getch();
	return rc;
}
