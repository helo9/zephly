#include <stdint.h>
#include <stddef.h>

/*
 * Defines.
 *
 * Contains one macro for each parameter that maps its name to its ID.
 * Example:
 * #define RATE_A 0
 */

// Integer parameters

{% for p in int_params -%}
#define {{ p.name }} {{ p.id }}
{% endfor %}

// Float parameters

{% for p in float_params -%}
#define {{ p.name }} {{ p.id }}
{% endfor %}

// Methods

uint32_t param_getu(uint16_t id);
void param_setu(uint16_t id, uint32_t val);

int32_t param_geti(uint16_t id);
void param_seti(uint16_t id, int32_t val);

float param_getf(uint16_t id);
void param_setf(uint16_t id, float val);

const char *param_name(uint16_t id);

int param_dump(const char *path);
int param_load(const char *path);
