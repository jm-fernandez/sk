#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "digits.h"
#include "sprite.h"
#include "log.h"


static const char digits_str[] = "0123456789";

digits_t* digits_create(const char* folder_path)
{
    digits_t* result = (digits_t*) malloc(sizeof(digits_t));
    if(result)
    {
        int i = 0;
        char file_path[256];

        memset(result, 0, sizeof(digits_t));

        for(; i < DIGITS_COUNT; ++i)
        {
            sprintf(file_path, "%s\\%c.bmp", folder_path, digits_str[i]);
            result->sprites[i] = create_bmp_sprite(file_path);
            if(!result->sprites[i])
            {
                char msg[256];
                sprintf(msg, "Error opening %s", file_path);
                log_record(msg);
                break;
            }
        }
        if(i != DIGITS_COUNT)
        {
            digits_free(result);
            result = NULL;
        }
    }
    return result;
}

void digits_free(digits_t* digits)
{
    int i = 0;
    for(; i < DIGITS_COUNT; ++i)
    {
        if(digits->sprites[i] != NULL)
        {
            sprite_free(digits->sprites[i]);
        }
    }
    free(digits);
}
