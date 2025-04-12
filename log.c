#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "log.h"

const char* log_file = NULL;

void log_initialize(const char* file)
{
    remove(file);
    log_file = file;
}

void log_deinitialize()
{
    log_file = NULL;
}

void log_record(const char* msg)
{
    FILE* f = fopen(log_file, "a");
    if(f)
    {
        fprintf(f, "%s\n", msg);
        fclose(f);
    }
}
