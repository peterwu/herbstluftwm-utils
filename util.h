#ifndef HERBSTLUFTWM_UTILS_UTIL_H
#define HERBSTLUFTWM_UTILS_UTIL_H

#include <bits/stdint-uintn.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

enum { LOW,
    NORMAL,
    CRITICAL };

struct notification_t {
    const char* app_name;
    const char* icon;
    const char* summary;
    const char* body;
    int urgency;
    int value;
    int timeout;
};

int pscanf(const char *path, const char *fmt, ...);
int pprintf(const char *path, const char *fmt, ...);

uint32_t notify_send(struct notification_t* n);
void notify_close(unsigned int id);

void systemctl_suspend();

#endif//HERBSTLUFTWM_UTILS_UTIL_H
