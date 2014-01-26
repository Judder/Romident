/*
      MAKEDAT v2.0 release 2

      Thierry Lescot, 1998.
*/

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <dirent.h>
#include <sys/types.h>
#include "unzip.h"
#include "defs.h"

char *wbuf;

typedef struct
   {
      short int game;
      char rom[ROMNAME_SIZE];
      unsigned int crc32;
      int size;
   } romid_t;

romid_t simple_entry;
FILE *gamf, *romf;

int ident(char *fn);
int adddat_file(char *fn, int size);
int adddat_zip(char *fn);
int adddat_dir(char *fn);

char old_game[80] = "\0";
int g_entry = 0, r_entry = 0;
int add_entry(char *game, char *rom, unsigned int crc32, int size)
{
   char record[80];

   if (size%32) return 1;

   strupr(rom);
   if ((strstr(rom, ".TXT"))||(strstr(rom, "READ"))||(strstr(rom, ".ME"))
      ||(strstr(rom, ".DOC"))||(strstr(rom, ".XLS"))) {
      return 1;
   };


   if (strcmp(game, old_game)!=0) {
      g_entry++;
      memset(old_game, 0, 80);
      sprintf(old_game, "%s", game);
      fwrite(old_game, 80, 1, gamf);
      printf("creating new entry for '%s', entry number %d\n", old_game, g_entry-1);
   };

   memset(&simple_entry, 0, sizeof(romid_t));
   simple_entry.game = g_entry - 1;
   sprintf(simple_entry.rom, "%s", rom);
   simple_entry.crc32 = crc32;
   simple_entry.size = size;
   fwrite(&simple_entry, sizeof(romid_t), 1, romf);
//   printf("rom entry added, game = %d, rom = '%s' (%d)\n", g_entry-1, rom, r_entry);
   r_entry++;
   return 0;
};

int open_datafile(char *fn)
{
   char *gamfile = "romident.gam";
   char *romfile = "romident.rom";
   char path[256];
   int i = strlen(fn);
   while ((fn[i-1]!='/')&&(fn[i-1]!='\\')) i--;
   sprintf(path, "%s", fn);
   sprintf(&path[i], "%s", gamfile);
   printf("GAM FILE = '%s'\n", path);
   if (!(gamf = fopen(path, "wb+"))) return 1;
   sprintf(&path[i], "%s", romfile);
   printf("ROM FILE = '%s'\n", path);
   if (!(romf = fopen(path, "wb+"))) {
      fclose(gamf);
      return 2;
   };
   getch();
   return 0;
}

int adddat_file(char *fn, int size)
{
   unsigned int crc;
   FILE *f;
   char dirname[256], filename[20];

   get_dirname(fn, dirname);
   get_filename(fn, filename);
   if (size == -1) {
//      printf("fpath = %s\n", fn);
      size = get_file_size(fn);
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
   f = fopen(fn, "rb");
   if (!f) {
      printf("Error, cannot open file '%s' !\n", fn);
      return 1;
   };
   fread(wbuf, 1, size, f);
   crc = crc32(0L, wbuf, size);
   fclose(f);
   free(wbuf);
//   printf("%-12s : CRC32 = 0x%08x : %s\n", filename, crc, dirname);
   add_entry(dirname, filename, crc, size);
   return 0;
}

int adddat_zip(char *fn)
{
   ZIP* zip;
   struct zipent* zipf;
   char zipname[256], filename[256];
   get_zipfilename(fn, zipname);
//   printf("Zip file to ident = '%s'\n", fn);
   if ((zip = openzip(fn)) == 0) {
      printf("Error, cannot open zip file '%s' !\n", fn);
      return 1;
   };
   while (zipf = readzip(zip)) {
      upper_case(zipf->name);
//      printf("%-12s : CRC32 = 0x%08x : %s\n",
//             zipf->name, zipf->crc32, zipname);
      get_filename(zipf->name, filename);
      add_entry(zipname, filename, zipf->crc32, zipf->uncompressed_size);
   };
   closezip(zip);
   return 0;
}

int adddat_dir(char *fn)
{
   DIR *dir;
   struct dirent *ent;
   char path[256], path2[256];
   sprintf(path, "%s\\*.*", fn);
   if ((dir = opendir(path)) != NULL) {
      /* ident all the files and directories within directory */
      while ((ent = readdir(dir)) != NULL) {
         sprintf(path2, "%s\\%s", fn, ent->d_name);
         ident(path2);
      }
      closedir (dir);
   } else {
      /* could not open directory */
      printf ("Could not open directory %s/*.*", path);
      return -1;
   }
}

int ident(char *fn)
{
   int l;
   char fncopy[256];
   int status = checkfile(fn);

   if (status == -1) {
        printf("'%s' path/file not found !\n", fn);
        return 1;
   };

//   printf("ident called with '%s'\n", fn);

   if (fn[strlen(fn)-1]!='.') {
      if (!opendir(fn)) {
         l = strlen(fn);
         if (l>4) {
            sprintf(fncopy, "%s", fn);
            upper_case(fncopy);
            if (strcmp(&fncopy[l-4], ".ZIP") == 0) {
               adddat_zip(fn);
            } else {
               adddat_file(fn, -1);
            }
         } else {
            adddat_file(fn, -1);
         };
      } else {
         adddat_dir(fn);
      };
   }
   return 0;
};

int main(int argc, char **argv)
{
   FILE *s;
   char str[256];
   int lastcommand = 0, i;

   printf("MAKEDAT v2.02\nThierry Lescot, 1998/2000.\n\n");

   if (!(s = fopen("MAKEDAT.CFG", "r"))) {
      printf("Error, i can't open MAKEDAT.CFG !\n");
      return 1;
   };

   if (i = open_datafile(argv[0])) {
      printf("Error, i can't open %s !\n", (i==1)? "ROMIDENT.GAM":"ROMIDENT.ROM");
      return 1;
   };

   fscanf(s, "%s", str);
   while (!feof(s))
   {
      if (!lastcommand) {
         if (strcmp(str, "ADD") == 0) lastcommand = 1;
         if (strcmp(str, "WAIT") == 0) lastcommand = 2;
         if (!lastcommand) {
            printf("Error in MAKEDAT.CFG (string = '%s') !\n", str);
         };
      } else {
         switch(lastcommand) {
            case 1 : // ADD
               printf("ident %s\n", str);
               ident(str);
               break;
            case 2 : // WAIT
               printf("%s\n", str);
               while (!kbhit());
               break;
          };
          lastcommand = 0;
      };
      fscanf(s, "%s", str);
   };

   fclose(s);
   fclose(gamf);
   fclose(romf);

   printf("total roms = %d\ntotal games = %d\n\n", r_entry, g_entry);

   return 0;
}
