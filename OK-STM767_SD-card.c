
#include "OK-STM767.h"
#include "OK-STM767_SD_card.h"



/* ============================================================================ */
/*			SD ī     FAT32       Լ 				*/
/* ============================================================================ */
unsigned char Get_long_filename(U08 file_number); // check long file name
void TFT_short_filename(U08 xchar,U08 ychar, U16 colorfore,U16 colorback);
void TFT_long_filename(U08 xchar,U08 ychar, U16 colorfore,U16 colorback);
unsigned short Unicode_to_KS(U16 unicode);	// convert Unicode(     ڵ ) to KS(      )
void Filename_arrange(U16 new, U16 old, U08 *FileNameBuffer); // arrange file name



/* ---------------------------------------------------------------------------- */
/*		      ϸ        (13*15=195 ڱ       )				*/
/* ---------------------------------------------------------------------------- */

unsigned char Get_long_filename(U08 file_number) /* check long file name */
{
  unsigned short i, n = 0;
  unsigned short dir_address, dir_number = 0;
  unsigned int	 dir_sector = 0;

  unsigned char ShortFileName[11];		// short file name buffer

  SD_read_sector(file_start_sector[file_number], SectorBuffer);	// read directory entry
  dir_address = file_start_entry[file_number]*32;

  if(SectorBuffer[dir_address+11] == 0x0F)	// *** long file name entry
    { dir_number = (SectorBuffer[dir_address] & 0x1F) + 1; // total number of directory entry

      if(dir_number > 16)			// if file name character > 13*15, return 2
        return 2;

      for( ; dir_address < 512; dir_address++)
	EntryBuffer[n++] = SectorBuffer[dir_address];

      if((dir_number*32) >= n)			// directory entry use 2 sectors
	{ dir_sector = file_start_sector[file_number];
	  SD_read_sector(++dir_sector, SectorBuffer);

	  i = 0;
	  for( ; n < 512; n++)
	    EntryBuffer[n] = SectorBuffer[i++];
	}

      dir_number = (EntryBuffer[0] & 0x1F)*32;	// last entry is short file name entry
      for(n = 0; n < 11; n++)
        ShortFileName[n] = EntryBuffer[dir_number++];

      extension = ShortFileName[8];		// get filename extension character
      extension <<= 8;
      extension += ShortFileName[9];
      extension <<= 8;
      extension += ShortFileName[10];
      return 1;					// if long file name, return 1
    }

  else if(SectorBuffer[dir_address+11] == 0x20)	// *** short file name entry
    { for( ; dir_address < 512; dir_address++)
	EntryBuffer[n++] = SectorBuffer[dir_address];

      for(n = 0; n < 11; n++)
        ShortFileName[n] = EntryBuffer[dir_number++];

      extension = ShortFileName[8];		// get filename extension character
      extension <<= 8;
      extension += ShortFileName[9];
      extension <<= 8;
      extension += ShortFileName[10];
      return 0;					// if short file name, return 0
    }

  else						// if file name error, return 3
    return 3;
}

/* ---------------------------------------------------------------------------- */
/*		ª      ϸ     						*/
/* ---------------------------------------------------------------------------- */

void TFT_short_filename(U08 xchar,U08 ychar, U16 colorfore,U16 colorback) /* display short filename */
{
  unsigned char  ch1, i;
  unsigned short ch2;

  Xcharacter = xchar;				// start position
  Ycharacter = ychar;

  foreground = colorfore;			// foreground color and background color
  background = colorback;

  for(i = 0; i < 11; i++ )			// convert upper case to lower case
    { if((EntryBuffer[i] >= 'A') && (EntryBuffer[i] <= 'Z'))
        EntryBuffer[i] += 0x20;	
    }

  for(i = 0; i < 11; i++)			// display 8.3 format
    { if(i == 8)
        TFT_English('.');

      ch1= EntryBuffer[i];
      if(ch1 < 0x80)				// English ASCII character
        {if(ch1 != 0x20)
	   TFT_English(ch1);
	}
      else					// Korean character
	{ ch2 = (ch1 << 8) + EntryBuffer[++i];
	  ch2 = KS_code_conversion(ch2);
	  TFT_Korean(ch2);
	}
    }
}

/* ---------------------------------------------------------------------------- */
/*		      ϸ     (40 ڱ    ǥ  )					*/
/* ---------------------------------------------------------------------------- */

void TFT_long_filename(U08 xchar,U08 ychar, U16 colorfore,U16 colorback) /* display long filename */
{
  unsigned char  ch1;
  unsigned short ch2, i, entrynumber, charnumber, newindex, oldindex;
  unsigned char  FileNameBuffer[512];

  Xcharacter = xchar;				// start position
  Ycharacter = ychar;

  foreground = colorfore;			// foreground color and background color
  background = colorback;

  entrynumber = EntryBuffer[0] & 0x1F;		// long entry number
  charnumber = (EntryBuffer[0] & 0x1F)*13;	// character number of file name
  oldindex = ((EntryBuffer[0] & 0x1F) - 0x01)*32;
  newindex = 0;

  for(i = 0; i < entrynumber; i++)		// arrange file name
    { Filename_arrange(newindex,oldindex,FileNameBuffer);
      newindex += 26;
      oldindex -= 32;
    }

  for(i = 0; i < charnumber*2; i++)		// display long file name format
    { if(i >= 40*2)				// limit length(40 character)
	{ TFT_English('~');
          TFT_English('1');
          return;
	}

      ch1 = FileNameBuffer[i];
      if(ch1 == 0xFF)
	return;

      if(ch1 < 0x80)				// English ASCII character
	TFT_English(FileNameBuffer[++i]);
      else					// Korean character
	{ ch2 = (ch1 << 8) + FileNameBuffer[++i];
	  ch2 = Unicode_to_KS(ch2);
	  TFT_Korean(ch2);
	}
    }
}

/* ---------------------------------------------------------------------------- */
/*		   ϸ     ڵ                 Ͽ    迭			*/
/* ---------------------------------------------------------------------------- */

void Filename_arrange(U16 newx, U16 oldx, U08 *FileNameBuffer) /* arrange file name */
{
  FileNameBuffer[newx+0]  = EntryBuffer[oldx+2];  // 1
  FileNameBuffer[newx+1]  = EntryBuffer[oldx+1];
  FileNameBuffer[newx+2]  = EntryBuffer[oldx+4];  // 2
  FileNameBuffer[newx+3]  = EntryBuffer[oldx+3];
  FileNameBuffer[newx+4]  = EntryBuffer[oldx+6];  // 3
  FileNameBuffer[newx+5]  = EntryBuffer[oldx+5];
  FileNameBuffer[newx+6]  = EntryBuffer[oldx+8];  // 4
  FileNameBuffer[newx+7]  = EntryBuffer[oldx+7];
  FileNameBuffer[newx+8]  = EntryBuffer[oldx+10]; // 5
  FileNameBuffer[newx+9]  = EntryBuffer[oldx+9];
  FileNameBuffer[newx+10] = EntryBuffer[oldx+15]; // 6
  FileNameBuffer[newx+11] = EntryBuffer[oldx+14];
  FileNameBuffer[newx+12] = EntryBuffer[oldx+17]; // 7
  FileNameBuffer[newx+13] = EntryBuffer[oldx+16];
  FileNameBuffer[newx+14] = EntryBuffer[oldx+19]; // 8
  FileNameBuffer[newx+15] = EntryBuffer[oldx+18];
  FileNameBuffer[newx+16] = EntryBuffer[oldx+21]; // 9
  FileNameBuffer[newx+17] = EntryBuffer[oldx+20];
  FileNameBuffer[newx+18] = EntryBuffer[oldx+23]; // 10
  FileNameBuffer[newx+19] = EntryBuffer[oldx+22];
  FileNameBuffer[newx+20] = EntryBuffer[oldx+25]; // 11
  FileNameBuffer[newx+21] = EntryBuffer[oldx+24];
  FileNameBuffer[newx+22] = EntryBuffer[oldx+29]; // 12
  FileNameBuffer[newx+23] = EntryBuffer[oldx+28];
  FileNameBuffer[newx+24] = EntryBuffer[oldx+31]; // 13
  FileNameBuffer[newx+25] = EntryBuffer[oldx+30];
}

/* ---------------------------------------------------------------------------- */
/*		 ѱ       ڵ带 KS         ڵ     ȯ				*/
/* ---------------------------------------------------------------------------- */

unsigned short Unicode_to_KS(U16 unicode)	/* convert Unicode(     ڵ ) to KS(      ) */
{
  unsigned char  cho = 0, joong = 0, jong = 0;
  unsigned short value;

  value = unicode - 0xAC00;                	//      ڵ忡   '  '    ش  ϴ           .

  jong  = value % 28;				//      ڵ带  ʼ ,  ߼ ,           и 
  joong = ((value - jong) / 28 ) % 21;
  cho   = ((value - jong) / 28 ) / 21;

  cho += 2;                           	 	//  ʼ  +       

  if(joong < 5)       joong += 3;		//  ߼  +       
  else if(joong < 11) joong += 5;
  else if(joong < 17) joong += 7;
  else                joong += 9;

  if(jong < 17) jong++;				//      +       
  else          jong += 2;

  return 0x8000 | (cho << 10) | ( joong << 5) | jong; //         ڵ 
}