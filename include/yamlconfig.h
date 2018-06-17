#ifndef YAML_CONFIG_H
#define YAML_CONFIG_H

#include <stdio.h>
#include <stdbool.h>

typedef struct yaml_config_ *yaml_config_t;
typedef void *yaml_config_iterator_t;

yaml_config_t yaml_config_new(void);
void yaml_config_free(yaml_config_t conf);

char const *yaml_config_strerror(yaml_config_t conf);

yaml_config_t yaml_config_lookup(yaml_config_t conf, char const *path);
bool yaml_config_load_file(yaml_config_t conf, char const *filename);

bool yaml_config_is_sequence(yaml_config_t c);
bool yaml_config_is_scalar(yaml_config_t c);
bool yaml_config_is_mapping(yaml_config_t c);


bool yaml_config_mapping_next(yaml_config_t conf, yaml_config_iterator_t *it,
                              char **key, yaml_config_t *child);

bool yaml_config_string(yaml_config_t c, char const *name,
                        char const **value, char const *def);

#endif
