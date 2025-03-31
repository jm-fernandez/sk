#ifndef LOG_H__
#define LOG_H__

void log_initialize(const char* file);
void log_deinitialize();
void log_record(const char* msg);


#endif // LOG_H__
