#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "log.h"

static FILE* log_file = NULL;

void log_initialize(const char* file)
{
    log_file = fopen(file, "w");
}

void log_deinitialize()
{
    fclose(log_file);
    log_file = NULL;
}

void log_record(const char* msg)
{
    if(log_file)
    {
        fprintf(log_file, "%s\n", msg);
        fflush(log_file);
    }
}
