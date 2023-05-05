#ifndef HERBSTLUFTWM_UTILS_UTIL_H
#define HERBSTLUFTWM_UTILS_UTIL_H

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

int pscanf(const char *path, const char *fmt, ...);
int pprintf(const char *path, const char *fmt, ...);

#endif//HERBSTLUFTWM_UTILS_UTIL_H
