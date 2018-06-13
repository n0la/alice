#ifndef YAML_CONFIG_H
#define YAML_CONFIG_H

#include <stdio.h>
#include <stdbool.h>

typedef struct yaml_config_ *yaml_config_t;

yaml_config_t yaml_config_new(void);
void yaml_config_free(yaml_config_t conf);

yaml_config_t yaml_config_lookup(yaml_config_t conf, char const *path);
bool yaml_config_load_file(yaml_config_t conf, char const *filename);

bool yaml_config_is_sequence(yaml_config_t c);
bool yaml_config_is_scalar(yaml_config_t c);
bool yaml_config_is_mapping(yaml_config_t c);

#endif
