#include <string.h>

#include "font.h"
#include "sprite.h"

static const char chars[] = "abcdefghijklmnopqrstuvwxyz0123456789";

typedef struct font_letter_by_letter_t_ {
    font_t font;
    sprite_t* letters[sizeof(chars) - 1];
} font_letter_by_letter_t;

static void font_lbl_free(font_t* font)
{
  font_letter_by_letter_t* font_lbl = (font_letter_by_letter_t*) font;
  if(font_lbl)
  {
      int i = 0;
      for(; i < sizeof(chars) - 1; ++i)
      {
        if(font_lbl->letters[i] != NULL)
        {
            sprite_free(font_lbl->letters[i]);
            font_lbl->letters[i] = NULL;
        }
      }

      free(font_lbl);
  }
}

static const sprite_t* font_lbl_get_letter(const font_t* font, char letter)
{
    sprite_t* result = NULL;
    const font_letter_by_letter_t* font_lbl = (const font_letter_by_letter_t*) font;
    if(font_lbl)
    {
        if(letter >= 'A' && letter <= 'Z')
        {
            result =  font_lbl->letters[letter - 'A'];
        }
        else if(letter >= 'a' && letter <= 'z')
        {
            result = font_lbl->letters[letter - 'a'];
        }
        else
        {
            result = font_lbl->letters[26 + letter - '0'];
        }
    }
    return result;
}

font_t* font_load(const char* folder)
{
    int i = 0;
    font_letter_by_letter_t* font = (font_letter_by_letter_t*) malloc(sizeof(font_letter_by_letter_t));
    if(font)
    {
        const int file_name_size = strlen(folder) + sizeof("\\x.bmp");
        char* file_name = (char*) malloc(file_name_size * sizeof(char));
        
        memset(font, 0, sizeof(font_letter_by_letter_t));

        if(file_name)
        {
            strcpy(file_name, folder);
            strcat(file_name, "\\x.bmp");

            for(; i < sizeof(chars) - 1; ++i)
            {
                file_name[file_name_size - 6] = chars[i];
                font->letters[i] = create_bmp_sprite(file_name);
                if(font->letters[i] == NULL)
                {
                    break;
                }
            }
            free(file_name);
        }
    }

    if(font != NULL && i < sizeof(chars) - 1)
    {
        font_lbl_free(&(font->font));
        font = NULL;
    }
    
    if(font != NULL)
    {
        font->font.free = font_lbl_free;
        font->font.get_letter = font_lbl_get_letter;
    }

    return &(font->font);
}
