#include "util.h"
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

int pscanf(const char *path, const char *fmt, ...) {
    FILE *fp;
    va_list ap;
    int n;

    if (!(fp = fopen(path, "r"))) {
        return -1;
    }

    va_start(ap, fmt);
    n = vfscanf(fp, fmt, ap);
    va_end(ap);
    fclose(fp);

    return (n == EOF) ? -1 : n;
}

int pprintf(const char *path, const char *fmt, ...) {
    FILE *fp;
    va_list ap;
    int n;

    if (!(fp = fopen(path, "w"))) {
        return -1;
    }

    va_start(ap, fmt);
    n = vfprintf(fp, fmt, ap);
    va_end(ap);
    fclose(fp);

    return (n == EOF) ? -1 : n;
}