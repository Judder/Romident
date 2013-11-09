/*
      ROMIDENT v2.0 release 2.1

      Thierry Lescot, 1998/99.
*/

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <dirent.h>
#include <sys/types.h>
#include "unzip.h"
#include "defs.h"

typedef struct
   {
      short int game;
      char rom[ROMNAME_SIZE];
      unsigned int crc32;
      int size;
   } romid_t;

romid_t simple_entry;
romid_t *rtab;
struct { char title[80]; } *gtab;
char *wbuf;

char unk = 0;

int r_entry = 0, g_entry = 0;

int load_datafile(char *fn)
{
   char *gamfile = "romident.gam";
   char *romfile = "romident.rom";
   char path[256];
   int i = strlen(fn);
   FILE *f;

   while ((fn[i-1]!='/')&&(fn[i-1]!='\\')) i--;
   sprintf(path, "%s", fn);
   sprintf(&path[i], "%s", gamfile);
   printf("GAM FILE = '%s'\n", path);
   if ((g_entry = get_file_size(path)) == -1) return 1;
   g_entry = g_entry / 80;
   printf("Games in database = %d games.\n", g_entry);
   if (!(f = fopen(path, "rb"))) return 1;
   // load games datas
   if (!(gtab = (void *)malloc(g_entry * 80))) {
      printf("Error: Not enough memory to load games datas !\n");
      return 1;
   };
   fread(gtab, 80, g_entry, f);
   fclose(f);

   sprintf(&path[i], "%s", romfile);
   printf("ROM FILE = '%s'\n", path);
   if ((r_entry = get_file_size(path)) == -1) return 1;
   r_entry = r_entry / sizeof(romid_t);
   printf("number of entries in rom file = %d\n", r_entry);
   if (!(f = fopen(path, "rb"))) return 2;
   // load roms datas
   if (!(rtab = (void *)malloc(r_entry * sizeof(romid_t)))) {
      printf("Error: Not enough memory to load games datas !\n");
      return 1;
   };
   fread(rtab, sizeof(romid_t), r_entry, f);
   fclose(f);
   
   return 0;
}

int romident(char *rom, unsigned int crc32, int size)
{   
   int i, f = 0;

   if (size%32) return -1;

   printf("%-12s [%08x] ", rom, crc32);
   for (i=0;i!=r_entry;i++) {
      if (rtab[i].crc32 == crc32) {
         if (f) printf("                        ");
         f++;
         printf("= %-12s from %s\n", rtab[i].rom, gtab[rtab[i].game].title);
      };
   };
   if (!f) {
      printf("NOT FOUND!\n");
      unk = 5;
   };
   return f;
};

int ident_crc(unsigned int crc32)
{   
   int i, f = 0;

   printf("Checking crc 0x%08x ... ", crc32);
   for (i=0;i!=r_entry;i++) {
      if (rtab[i].crc32 == crc32) {
         if (f) printf("                  ");
         f++;
         printf("%-12s from %s\n", rtab[i].rom, gtab[rtab[i].game].title);
      };
   };
   if (!f) {
      printf("NOT FOUND IN DATABASE!\n");
      unk = 5;
   };
   return f;
};


int ident_file(char *path, char *fn, int size)
{
   unsigned int crc;
   FILE *f;
   char fpath[256];

   sprintf(fpath, "%s\\%s", path, fn);
   if (size == -1) {
      //printf("fpath = %s\n", fpath);
      size = get_file_size(fpath);
      if (size < 1) {
         printf("Error, '%s' size incorrect !\n", fn);
         return 1;
      };
   };
   wbuf = (char *)malloc(size);
   if (!wbuf) {
      printf("Error, not enough memory to '%s' the file into memory !\n", fn);
      return 1;
   };
   f = fopen(fpath, "rb");
   if (!f) {
      printf("Error, cannot open file '%s' !\n", fpath);
      return 1;
   };
   fread(wbuf, 1, size, f);
   crc = crc32(0L, wbuf, size);
   fclose(f);
   free(wbuf);
   romident(fn, crc, size);
   return 0;
}

int ident_zip(char *fn)
{
   ZIP* zip;
   struct zipent* zipf;
   printf("Zip file to ident = '%s'\n", fn);
   if ((zip = openzip(fn)) == 0) {
      printf("Error, cannot open zip file '%s' !\n", fn);
      return 1;
   };
   while (zipf = readzip(zip)) {
      upper_case(zipf->name);
      romident(zipf->name, zipf->crc32, zipf->uncompressed_size);
   };
   closezip(zip);
   return 0;
}

int ident_dir(char *fn)
{
   int count,i = 0;
   struct dirent **files;
   int file_select = NULL;
   count = scandir(fn, &files, file_select, alphasort);
   if (count > 0) {
      for (i = 0; i < count; ++i) {
         ident(files[i]->d_name);
      }
   }
   return 0;
}

int ident(char *fn)
{
   int l;

   if (!is_directory(fn)) {
      l = strlen(fn);
      if (l>4) {
         if (strcmp(&fn[l-4], ".ZIP") == 0) {
            ident_zip(fn);
         } else {
            ident_file(".", fn, -1);
         }
      } else {
         ident_file(".", fn, -1);
      };
   } else {
      ident_dir(fn);
   };
};

int main(int argc, char **argv)
{
   int i, nf;

   printf("ROMIDENT v2.1\nThierry Lescot, 1998/99.\n\n");

   load_datafile(argv[0]);

   printf("DAT file revision %d.\n", g_entry);

   if (argc<2) {
      printf("Error, specify at least one file name !\n");
      return 1;
   };

   for (nf=1;nf<argc;nf++) {
      if (argv[nf][0] == '-') {
         switch(argv[nf][1]) {
            case '&':
               ident_crc(hex2int(&argv[nf][2]));
               break;
         };
      } else {
         if ((i = checkfile(argv[nf])) == -1) {
            printf("Error, '%s' doesn't exist !\n", argv[1]);
            return 1;
         };
         ident(argv[nf]);
      };
   };

   return unk;
}
