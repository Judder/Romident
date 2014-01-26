/*
      ROMIDENT v2.0 release 2.1

      Thierry Lescot, 1998/99.
*/

#include <stdio.h>
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
//   printf("GAM FILE = '%s'\n", path);
   if ((g_entry = get_file_size(path)) == -1) return 1;
   g_entry = g_entry / 80;
//   printf("Games in database = %d games.\n", g_entry);
   if (!(f = fopen(path, "rb"))) return 1;
   // load games datas
   if (!(gtab = (void *)malloc(g_entry * 80))) {
      printf("Error: Not enough memory to load games datas !\n");
      return 1;
   };
   fread(gtab, 80, g_entry, f);
   fclose(f);

   sprintf(&path[i], "%s", romfile);
//   printf("ROM FILE = '%s'\n", path);
   if ((r_entry = get_file_size(path)) == -1) return 1;
   r_entry = r_entry / sizeof(romid_t);
//   printf("number of entries in rom file = %d\n", r_entry);
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


int main(int argc, char **argv)
{
   int i, nf;
   FILE *f;

   printf("RIEXPORT for ROMIDENT v2.1\nThierry Lescot, 1998/99.\n\n");

   load_datafile(argv[0]);

   printf("DAT file revision %d.\n", g_entry);

   f=fopen("ri_games.txt","w+");
   fprintf(f, "gnumber;name\n");
   for (i=0;i!=g_entry;i++) {
      fprintf(f, "%d;\"%s\"\n", i, gtab[i].title);
   };
   fclose(f);

   f=fopen("ri_roms.txt","w+");
   fprintf(f, "gnumber;name;crc32;size\n");
   for (i=0;i!=r_entry;i++) {
      fprintf(f, "%d;\"%s\";\"%08x\";%d\n", rtab[i].game, rtab[i].rom,
               rtab[i].crc32, rtab[i].size);
   };
   fclose(f);

   return unk;
}
