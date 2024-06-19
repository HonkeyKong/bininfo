/*******************************************************************************
  BININFO - Program that gives saturn program information

  (c) Copyright 2003 Theo Berkau(cwx@softhome.net)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*******************************************************************************/

// Detects whether a program uses SBL/SGL(and lists with libraries are used,
// along with version) using a pretty crude method

#include "main.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#define PROG_NAME "BININFO"
#define VER_NAME "0.11"
#define COPYRIGHT_YEAR "2024"

//////////////////////////////////////////////////////////////////////////////

void ProgramUsage()
{
   printf("%s v%s - by Cyber Warrior X/HonkeyKong (c)%s\n", PROG_NAME, VER_NAME, COPYRIGHT_YEAR);
   printf("usage: %s <filename>\n", PROG_NAME);
   exit (1);
}

//////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
   FILE *fp;
   char *filename;
   unsigned long f_size=0;
   unsigned char *buffer;
   unsigned long counter=0;
   unsigned long temp_long=0;
   unsigned long temp_long2=0;
   unsigned long i;
   unsigned long first_func=0;
   // bool sgl_found=false;
   // bool sglcd_found=false;

   if (argc < 2)
   {
      ProgramUsage();
   }

   printf("%s v%s - by Cyber Warrior X/HonkeyKong (c)%s\n", PROG_NAME, VER_NAME, COPYRIGHT_YEAR);

   filename = argv[1];

   if ((fp = fopen(filename, "rb")) == NULL)
   {
      printf("Unable to open file: %s\n", filename);
      exit(1);
   }

   fseek(fp, 0, SEEK_END);
   f_size = ftell(fp);
   fseek(fp, 0, SEEK_SET);

   // allocate buffer for file
   if ((buffer = (unsigned char *)malloc(f_size + 300)) == NULL)
   {
      printf("Unable to allocate buffer\n");
      fclose(fp);
      exit(1);
   }

   // read in file and close it
   memset(buffer, 0, f_size + 300);
   fread((void *)buffer, 1, f_size, fp);
   fclose(fp);

   // Let's first of all figure out whether this is a complete program
   // or a dynamically linked program

   temp_long = (buffer[0] << 24) +
               (buffer[1] << 16) +
               (buffer[2] << 8) +
                buffer[3];

   if ((temp_long >= 0x0600000 && temp_long < 0x06100000) ||
       (temp_long >= 0x0020000 && temp_long < 0x00300000))
   {
      // It's a dynamically linked program. Keep grabing longs from file
      // until it's detected that it's not an address(just check to see if the
      // first byte is 0x2F). Assume that it's the address the -lowest-
      // pointer(doesn't have to be the first in the list was refering to and
      // subtract it's offset from the start of the program
      printf("dynamically linked program detected\n");

      for (i = 0; i < (f_size / 4); i++)
      {
         temp_long2 = (buffer[4 + (i * 4)] << 24) +
                      (buffer[4 + (i * 4) + 1] << 16) +
                      (buffer[4 + (i * 4) + 2] << 8) +
                       buffer[4 + (i * 4) + 3];

         if ((temp_long2 & 0xFF000000) == 0x2F000000 ||
             (temp_long2 & 0xFF000000) == 0x4F000000 ||
             (temp_long2 & 0xFF000000) == 0x7F000000)
         {
            first_func = 4 + (i * 4);
         //   printf("first function at offset %08x\n", first_func);
            break;
         }
         else
         {
            // make sure it's a real offset
            if ((temp_long2 >= 0x0600000 && temp_long2 < 0x06100000) ||
                (temp_long2 >= 0x0020000 && temp_long2 < 0x00300000))
            {
               if (temp_long2 < temp_long)
                  temp_long = temp_long2;
            }
         }
      }

      printf("start address = %08lx\n", temp_long - first_func);
   }
   else
   {
      // it's an actual program. Haven't quite sorted out how to detect
      // the start of the program yet
      printf("statically linked program detected\n");

   }

   // Now search file for any SBL strings
   while((counter + 300) < f_size)
   {
      if (memcmp(buffer + counter, sega_adp_ident, sega_adp_size) == 0)
      {
         printf("ADPCM library detected\n");
         counter += sega_adp_size;
      }
      else if(memcmp(buffer + counter, "BUP Version", 11) == 0)
      {
         unsigned char major_ver;
         unsigned char minor_ver;

         major_ver = buffer[counter + 12] - 0x30;
         minor_ver = ((buffer[counter + 14] - 0x30) * 10) + (buffer[counter + 15] - 0x30);

         printf("Backup library v%d.%d detected at 0x%lX\n", major_ver, minor_ver, counter);
         counter += 28;
      }
      else if(memcmp(buffer + counter, "CPK Version", 11) == 0)
      {
         unsigned char major_ver;
         unsigned char minor_ver;

         major_ver = buffer[counter + 12] - 0x30;
         minor_ver = ((buffer[counter + 14] - 0x30) * 10) + (buffer[counter + 15] - 0x30);

         printf("Cinepak library v%d.%d detected at 0x%lX\n", major_ver, minor_ver, counter);
         counter += 28;
      }
      else if (memcmp(buffer + counter, "GFS_SBL Version", 15) == 0)
      {
         unsigned char major_ver;
         unsigned char minor_ver;

         major_ver = buffer[counter + 16] - 0x30;
         minor_ver = ((buffer[counter + 18] - 0x30) * 10) + (buffer[counter + 19] - 0x30);

         printf("GFS(SBL) library v%d.%d detected at 0x%lX\n", major_ver, minor_ver, counter);
         counter += 32;
      }
      if (memcmp(buffer + counter, "GFS_SGL Version", 15) == 0)
      {
         unsigned char major_ver;
         unsigned char minor_ver;

         major_ver = buffer[counter + 16] - 0x30;
         minor_ver = ((buffer[counter + 18] - 0x30) * 10) + (buffer[counter + 19] - 0x30);

         printf("GFS(SGL) library v%d.%d detected at 0x%lX\n", major_ver, minor_ver, counter);
         counter += 32;
      }
      else if (memcmp(buffer + counter, sega_mpg_ident, sega_mpg_size) == 0)
      {
         unsigned char major_ver;
         unsigned char minor_ver;

         major_ver = buffer[counter + 12] - 0x30;
         minor_ver = ((buffer[counter + 14] - 0x30) * 10) + (buffer[counter + 15] - 0x30);

         printf("MPEG library v%d.%d detected at 0x%lX\n", major_ver, minor_ver, counter);
         counter += sega_mpg_size;
      }
      else if(memcmp(buffer + counter, "PCM Version", 11) == 0)
      {
         unsigned char major_ver;
         unsigned char minor_ver;

         major_ver = buffer[counter + 12] - 0x30;
         minor_ver = ((buffer[counter + 14] - 0x30) * 10) + (buffer[counter + 15] - 0x30);

         printf("PCM library v%d.%d detected at 0x%lX\n", major_ver, minor_ver, counter);
         counter += 28;
      }
      else if (memcmp(buffer + counter, sega_per_ident, sega_per_size) == 0)
      {
         printf("Peripheral library detected\n");
         counter += sega_per_size;
      }
      else if (memcmp(buffer + counter, sega_scl_ident, sega_scl_size) == 0)
      {
         if (memcmp(buffer + counter + sega_scl_size + 4, sega_scl_ident2, sega_scl_size2) == 0)
         {
            printf("SCL library detected\n");
            counter += sega_scl_size + 4 + sega_scl_size2;
         }
         else if (memcmp(buffer + counter + sega_scl_size + 6, sega_scl_ident2, sega_scl_size2) == 0)
         {
            printf("SCL library detected\n");
            counter += sega_scl_size + 6 + sega_scl_size2;
         }
         else
         {
            counter += 1;
         }
      }
      else if (memcmp(buffer + counter, sega_spr_ident, sega_spr_size) == 0)
      {
         printf("Sprite library detected\n");
         counter += sega_spr_size;
      }
      else if(memcmp(buffer + counter, "STM_SBL Version", 15) == 0)
      {
         unsigned char major_ver;
         unsigned char minor_ver;

         major_ver = buffer[counter + 16] - 0x30;
         minor_ver = ((buffer[counter + 18] - 0x30) * 10) + (buffer[counter + 19] - 0x30);

         printf("Streaming(SBL) library v%d.%d detected at 0x%lX\n", major_ver, minor_ver, counter);
         counter += 32;
      }
      else if(memcmp(buffer + counter, "STM_SGL Version", 15) == 0)
      {
         unsigned char major_ver;
         unsigned char minor_ver;

         major_ver = buffer[counter + 16] - 0x30;
         minor_ver = ((buffer[counter + 18] - 0x30) * 10) + (buffer[counter + 19] - 0x30);

         printf("Streaming(SGL) library v%d.%d detected at 0x%lX\n", major_ver, minor_ver, counter);
         counter += 32;
      }
      else if(memcmp(buffer + counter, "SYS Version", 11) == 0)
      {
         unsigned char major_ver;
         unsigned char minor_ver;

         major_ver = buffer[counter + 12] - 0x30;
         minor_ver = ((buffer[counter + 14] - 0x30) * 10) + (buffer[counter + 15] - 0x30);

         printf("System library v%d.%d detected at 0x%lX\n", major_ver, minor_ver, counter);
         counter += 28;
      }
      else
      {
         counter += 1;
      }
   }

   free(buffer);

   return 0;
}

//////////////////////////////////////////////////////////////////////////////

