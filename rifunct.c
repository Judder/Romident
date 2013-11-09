#include <stdio.h>
#include <string.h>
#include <dirent.h>

typedef struct
   {
      short int game;
      char rom[13];
      unsigned int crc32;
      unsigned int crcs[6];
      int size;
   } ex_romid_t;

int checkfile(char *fn)
{
   //struct ffblk f;
   //int st = findfirst(fn, &f, FA_ARCH|FA_DIREC|FA_RDONLY);
   //int st = findfirst(fn, &f);
   //if (st) return -1;
   //return f.ff_attrib;

   FILE *fp;
   fp = fopen(fn, "r");
   if (fp == NULL) {
      return -1;
   } else {
      return 1;
   }

   return access(fn);
};

int is_directory(char *fn)
{
   struct dirent **files;
   int file_select = NULL;
   if (scandir(fn, &files, file_select, alphasort) > 0) return 1;
   return 0;
};

void upper_case(char *str)
{
   int i, l = strlen(str);
   for (i=0;i!=l;i++) if ((str[i]>='a')&&(str[i]<='z')) str[i]-=32;
};

void upper_case_2(char *str)
{
   int i, l = strlen(str);
   for (i=0;i!=l;i++) {
      if ((str[i]>='a')&&(str[i]<='z')) str[i]-=32;
      if (str[i] == '_') str[i] = ' ';
   };
};
/*
int get_file_size(char *fn)
{
   struct find_t f;
   if (_dos_findfirst(fn, _A_NORMAL|_A_RDONLY, &f)) return -1;
   return f.size;
};
*/
int get_file_size(char *fn)
{
   FILE *fp;
   long file_size;
   fp = fopen(fn, "r");
   if (fp == NULL) return -1;
   if (fseek(fp, 0, SEEK_END) != 0) return -1;
   file_size = ftell(fp);
   fclose(fp);
   return file_size;
};

void get_zipfilename(char *s, char *d)
{
   int i, ss, ee = strlen(s);
   while (s[ee]!='.') ee--;
   ss = ee - 1;
   while ((s[ss]!='/')&&(s[ss]!='\\')&&(ss)) ss--;
   if ((s[ss]=='/')||(s[ss]=='\\')) ss++;
   for (i=0;i!=(ee-ss);i++) d[i] = s[ss + i];
   d[i] = 0;
};

void get_dirname(char *s, char *d)
{
   int i, ss, ee = strlen(s);
   while ((s[ee]!='/')&&(s[ee]!='\\')) ee--;
   ss = ee - 1;
   while ((s[ss]!='/')&&(s[ss]!='\\')&&(ss)) ss--;
   if ((s[ss]=='/')||(s[ss]=='\\')) ss++;
   for (i=0;i!=(ee-ss);i++) d[i] = s[ss + i];
   d[i] = 0;
};

void get_filename(char *s, char *d)
{
   int i, ss, ee = strlen(s);
   ss = ee - 1;
   while ((s[ss]!='/')&&(s[ss]!='\\')&&(ss)) ss--;
   if ((s[ss]=='/')||(s[ss]=='\\')) ss++;
   for (i=0;i!=(ee-ss);i++) d[i] = s[ss + i];
   d[i] = 0;
};

int hex2int(char *crcstr)
{
   int s = 0, i, v = 0, m = 1;
   char d;
   do {
      s++;
   } while ((crcstr[s]!=0)&&(crcstr[s]!=' '));
   s--;
   for (i=s;i>=0;i--) {
      if (crcstr[i]>='a') crcstr[i]-=32;
      d = crcstr[i];
      if ((d>='0') && (d<='9')) v+=m*(int)(d-'0');
      if ((d>='A') && (d<='F')) v+=m*(10+(int)(d-'A'));
      m = m * 16;
   };
   return v;
};

