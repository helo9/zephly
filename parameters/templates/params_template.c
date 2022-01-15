#include "params.h"
#include <stdio.h>

#define NPARAMS {{ nparams }}
static uint32_t parameter_store[NPARAMS];

uint32_t param_getu(uint16_t id) {
    return parameter_store[id];
}

void param_setu(uint16_t id, uint32_t val) {
    parameter_store[id] = val;
}

int32_t param_geti(uint16_t id) {

}

void param_seti(uint16_t id, int32_t val) {

}

float param_getf(uint16_t id) {

}

void param_setf(uint16_t id, float val) {

}

const char *param_name(uint16_t id) {
    switch (id) {
        {%- for p in int_params %}
        case {{ p.name }}:
            return "{{ p.name }}";
        {%- endfor -%}
        {%- for p in float_params %}
        case {{ p.name }}:
            return "{{ p.name }}";
        {%- endfor %}
        default:
            return NULL;
    }
}

int param_dump(const char *path) {
    // This needs to be adapted to zephyr.
    // Also really unsafe because of no error checking.

    FILE *f = fopen(path, "w+");
    fwrite((const void *)&parameter_store, sizeof(uint32_t), NPARAMS, f);
    fflush(f);
    fclose(f);
}

int param_load(const char *path) {
    // Same as above.

    FILE *f = fopen(path, "w+");
    fread((void *)&parameter_store, sizeof(uint32_t), NPARAMS, f);
    fclose(f);
}
