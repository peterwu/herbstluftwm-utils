#include "ibus.h"
#include <stdio.h>
#include <string.h>

#define RESULT_FMT "ime\t%s"

static void print_global_engine(IBusBus *bus){
    IBusEngineDesc *global_engine = ibus_bus_get_global_engine(bus);
    const gchar *name = NULL;
    name = ibus_engine_desc_get_name(global_engine);

    char result[8];

    if (!strcmp(name, "libpinyin")) {
        sprintf(result, RESULT_FMT, "zh");
    } else if (!strcmp(name, "xkb:us::eng")){
        sprintf(result, RESULT_FMT, "en");
    } else {
        goto finish;
    }

    printf("%s\n", result);

    finish:
    g_object_unref(global_engine);
}

static void global_engine_changed_cb(IBusBus *bus) {
    print_global_engine(bus);
}

int main() {
    IBusBus *bus;

    ibus_init();
    bus = ibus_bus_new();

    print_global_engine(bus);

    g_signal_connect(bus, "global-engine-changed",
                     G_CALLBACK(global_engine_changed_cb), bus);

    ibus_bus_set_watch_ibus_signal(bus, TRUE);

    ibus_main();

    g_object_unref(bus);
    return 0;
}
