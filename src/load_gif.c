/*
 * GIF Loader
 * by Paul Bartrum
 *
 * Comment by Eric Stevens (tome@eastevens.com):
 *
 * This file has been modified to work with Allegro 4.0.3 that comes with
 * DJGPP 2.0.3 for the use in compiling for ToME 2.2.7. This modification
 * changes a variable name to one that is not in conflict with Allegro
 * variable name. The variable 'empty_string' was changed to 'empty_str'.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <allegro.h>
#include <string.h>

int _color_load_depth(int depth);

struct LZW_STRING
{
	short base;
	char new;
	short length;
};

PACKFILE *f;
int empty_str, curr_bit_size, bit_overflow;
int bit_pos, data_pos, data_len, entire, code;
int cc, string_length, i, bit_size;
unsigned char string[4096];
struct LZW_STRING str[4096];
BITMAP *bmp;
int image_x, image_y, image_w, image_h, x, y;
int interlace;


void clear_table(void)
{
	empty_str = cc + 2;
	curr_bit_size = bit_size + 1;
	bit_overflow = 0;
}


void get_code(void)
{
	if (bit_pos + curr_bit_size > 8)
	{
		if (data_pos >= data_len)
		{
			data_len = pack_getc(f);
			data_pos = 0;
		}
		entire = (pack_getc(f) << 8) + entire;
		data_pos ++;
	}
	if (bit_pos + curr_bit_size > 16)
	{
		if (data_pos >= data_len)
		{
			data_len = pack_getc(f);
			data_pos = 0;
		}
		entire = (pack_getc(f) << 16) + entire;
		data_pos ++;
	}
	code = (entire >> bit_pos) & ((1 << curr_bit_size) - 1);
	if (bit_pos + curr_bit_size > 8)
		entire >>= 8;
	if (bit_pos + curr_bit_size > 16)
		entire >>= 8;
	bit_pos = (bit_pos + curr_bit_size) % 8;
	if (bit_pos == 0)
	{
		if (data_pos >= data_len)
		{
			data_len = pack_getc(f);
			data_pos = 0;
		}
		entire = pack_getc(f);
		data_pos ++;
	}
}


void input_string(int num)
{
	if (num < cc)
	{
		string_length = 1;
		string[0] = str[num].new;
	}
	else
	{
		i = str[num].length;
		string_length = i;
		while (i > 0)
		{
			i --;
			string[i] = str[num].new;
			num = str[num].base;
		}
		/* if(num != -1) **-{[ERROR]}-** */
	}
}


void output_string(void)
{
	for (i = 0; i < string_length; i ++)
	{
		putpixel(bmp, x, y, string[i]);
		x ++;
		if (x >= image_x + image_w)
		{
			x = image_x;
			y += interlace;
			if (interlace)
			{
				if (y >= image_y + image_h)
				{
					if (interlace == 8 && (y - image_y) % 8 == 0)
					{
						interlace = 8;
						y = image_y + 4;
					}
					else if (interlace == 8 && (y - image_y) % 8 == 4)
					{
						interlace = 4;
						y = image_y + 2;
					}
					else if (interlace == 4)
					{
						interlace = 2;
						y = image_y + 1;
					}
				}
			}
		}
	}
}


/* load_gif:
 *  Loads a 2-256 colour GIF file onto a bitmap, returning the bitmap
 *  structure and storing the pallete data in the specified pallete (this
 *  should be an array of at least 256 RGB structures).
 */
BITMAP *load_gif (char *filename, RGB *pal)
{
	int width, height, depth;
	int old;
	BITMAP *bmp2;
	int dest_depth;

	f = pack_fopen(filename, F_READ);
	if (!f) /* can't open file */
		return NULL;

	i = pack_mgetw(f) << 8;
	i += pack_getc(f);
	if (i != 0x474946) /* is it really a GIF? */
	{
		pack_fclose(f);
		return NULL;
	}
	pack_fseek(f, 3);  /* skip version */

	width = pack_igetw(f);
	height = pack_igetw(f);

	bmp = create_bitmap_ex(8, width, height);
	if (bmp == NULL)
	{
		pack_fclose(f);
		return NULL;
	}
	clear(bmp);

	i = pack_getc(f);
	if (i & 128) /* no global colour table? */
		depth = (i & 7) + 1;
	else
		depth = 0;

	pack_fseek(f, 2); 	/* skip background colour and aspect ratio */

	if (pal && depth) /* only read palette if pal and depth are not 0 */
	{
		for (i = 0; i < (1 << depth); i ++)
		{
			pal[i].r = pack_getc(f) / 4;
			pal[i].g = pack_getc(f) / 4;
			pal[i].b = pack_getc(f) / 4;
		}
	}
	else
		if (depth)
			pack_fseek(f, (1 << depth) * 3);

	do
	{
		i = pack_getc(f);
		switch (i)
		{
		case 0x2C:  /* Image Descriptor */
			image_x = pack_igetw(f);
			image_y = pack_igetw(f);  /* individual image dimensions */
			image_w = pack_igetw(f);
			image_h = pack_igetw(f);

			i = pack_getc(f);
			if (i & 64)
				interlace = 8;
			else
				interlace = 1;

			if (i & 128)
			{
				depth = (i & 7) + 1;
				if (pal)
				{
					for (i = 0; i < (1 << depth); i ++)
					{
						pal[i].r = pack_getc(f) / 4;
						pal[i].g = pack_getc(f) / 4;
						pal[i].b = pack_getc(f) / 4;
					}
				}
				else
					pack_fseek(f, (1 << depth) * 3);
			}

			/* lzw stream starts now */
			bit_size = pack_getc(f);
			cc = 1 << bit_size;

			/* initialise string table */
			for (i = 0; i < cc; i ++)
			{
				str[i].base = -1;
				str[i].new = i;
				str[i].length = 1;
			}

			/* initialise the variables */
			bit_pos = 0;
			data_len = pack_getc(f);
			data_pos = 0;
			entire = pack_getc(f);
			data_pos ++;
			string_length = 0;
			x = image_x;
			y = image_y;

			/* starting code */
			clear_table();
			get_code();
			if (code == cc)
				get_code();
			input_string(code);
			output_string();
			old = code;

			while (TRUE)
			{
				get_code();

				if (code == cc)
				{
					/* starting code */
					clear_table();
					get_code();
					input_string(code);
					output_string();
					old = code;
				}
				else if (code == cc + 1)
				{
					break;
				}
				else if (code < empty_str)
				{
					input_string(code);
					output_string();

					if (bit_overflow == 0)
					{
						str[empty_str].base = old;
						str[empty_str].new = string[0];
						str[empty_str].length = str[old].length + 1;
						empty_str ++;
						if (empty_str == (1 << curr_bit_size))
							curr_bit_size ++;
						if (curr_bit_size == 13)
						{
							curr_bit_size = 12;
							bit_overflow = 1;
						}
					}

					old = code;
				}
				else
				{
					input_string(old);
					string[str[old].length] = string[0];
					string_length ++;

					if (bit_overflow == 0)
					{
						str[empty_str].base = old;
						str[empty_str].new = string[0];
						str[empty_str].length = str[old].length + 1;
						empty_str ++;
						if (empty_str == (1 << curr_bit_size))
							curr_bit_size ++;
						if (curr_bit_size == 13)
						{
							curr_bit_size = 12;
							bit_overflow = 1;
						}
					}

					output_string();
					old = code;
				}
			}
			break;
		case 0x21:  /* Extension Introducer */
			i = pack_getc(f);
			if (i == 0xF9) /* Graphic Control Extension */
			{
				pack_fseek(f, 1);  /* skip size (it's 4) */
				i = pack_getc(f);
				if (i & 1) /* is transparency enabled? */
				{
					pack_fseek(f, 2);
					pack_getc(f);  /* transparent colour */
				}
				else
					pack_fseek(f, 3);
			}
			i = pack_getc(f);
			while (i) /* skip Data Sub-blocks */
			{
				pack_fseek(f, i);
				i = pack_getc(f);
			}
			break;
		case 0x3B:  /* Trailer - end of data */
			pack_fclose(f);

			/* convert to correct colour depth */
			dest_depth = _color_load_depth(8);

			if (dest_depth != 8)
			{
				bmp2 = create_bitmap_ex(dest_depth, bmp->w, bmp->h);
				if (!bmp2)
				{
					destroy_bitmap(bmp);
					return NULL;
				}

				select_palette(pal);
				blit(bmp, bmp2, 0, 0, 0, 0, bmp->w, bmp->h);
				unselect_palette();

				destroy_bitmap(bmp);
				bmp = bmp2;
			}

			return bmp;
		}
	}
	while (TRUE);

	/* this is never executed but DJGPP complains if you leave it out */
	return NULL;
}
