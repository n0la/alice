#define _GNU_SOURCE
#include "yamlconfig.h"
#include <yaml.h>

struct yaml_config_
{
    yaml_document_t *doc;
    yaml_node_t *root;
    bool weak;
    char *error;
};

yaml_config_t yaml_config_new(void)
{
    return calloc(1, sizeof(struct yaml_config_));
}

void yaml_config_free(yaml_config_t c)
{
    if (c == NULL) {
        return;
    }

    if (!c->weak) {
        yaml_document_delete(c->doc);
        c->doc = NULL;
    }

    free(c->error);
    free(c);
}

yaml_config_t yaml_config_lookup(yaml_config_t conf, char const *path)
{
    yaml_node_t *node = NULL;
    bool done = false;
    char *part = NULL, *save = NULL, *token = NULL, *haystack = NULL;
    yaml_config_t ret = NULL;

    if (conf == NULL || path == NULL) {
        return NULL;
    }

    node = conf->root;
    part = strdup(path);
    haystack = part;

    while (!done) {
        token = strtok_r(haystack, "/.", &save);
        haystack = NULL;
        if (token == NULL) {
            break;
        }

        if (node->type == YAML_SCALAR_NODE) {
            if (strcmp(node->data.scalar.value, part) == 0) {
                break;
            } else {
                node = NULL;
                break;
            }
        } else if (node->type == YAML_MAPPING_NODE) {
            yaml_node_pair_t *it = NULL;
            yaml_node_t *found = NULL;

            for (it = node->data.mapping.pairs.start;
                 it < node->data.mapping.pairs.top;
                 it++) {
                yaml_node_t *k = yaml_document_get_node(conf->doc, it->key);
                yaml_node_t *v = yaml_document_get_node(conf->doc, it->value);

                if (strcmp(k->data.scalar.value, token) == 0) {
                    found = v;
                    break;
                }
            }

            if (found != NULL) {
                node = found;
            } else {
                node = NULL;
                break;
            }
        } else if (node->type == YAML_SEQUENCE_NODE) {
            node = NULL;
            break;
        }
    }

    free(part);

    if (node == NULL) {
        return NULL;
    }

    ret = yaml_config_new();
    if (ret == NULL) {
        return NULL;
    }

    ret->doc = conf->doc;
    ret->root = node;
    ret->weak = true;

    return ret;
}

char const *yaml_config_strerror(yaml_config_t conf)
{
    if (conf == NULL) {
        return "argument error";
    }

    return (conf->error == NULL ? "no error" : conf->error);
}

static bool yaml_config_load(yaml_config_t conf, FILE *f)
{
    yaml_parser_t parser;
    yaml_event_t ev;
    bool done = false;
    bool ret = false;

    if (conf->doc != NULL) {
        goto fail;
    }

    if (conf->doc == NULL) {
        conf->doc = calloc(1, sizeof(yaml_document_t));
        if (conf->doc == NULL) {
            goto fail;
        }
    }

    yaml_parser_initialize(&parser);
    yaml_parser_set_input_file(&parser, f);

    if (!yaml_parser_load(&parser, conf->doc)) {
        free(conf->error);
        asprintf(&conf->error, "offset: %d: %s",
                 parser.problem_offset, parser.problem
            );
        goto fail;
    }

    conf->root = yaml_document_get_root_node(conf->doc);
    if (conf->root == NULL) {
        yaml_document_delete(conf->doc);
        conf->doc = NULL;
        goto fail;
    }

    ret = true;

fail:

    yaml_parser_delete(&parser);

    return ret;
}

bool yaml_config_load_file(yaml_config_t conf, char const *filename)
{
    bool ret = false;
    FILE *f = NULL;

    if (conf == NULL || filename == NULL) {
        return false;
    }

    f = fopen(filename, "r");
    if (f == NULL) {
        return false;
    }

    ret = yaml_config_load(conf, f);
    fclose(f);

    return ret;
}

bool yaml_config_is_sequence(yaml_config_t c)
{
    if (c == NULL || c->root == NULL) {
        return false;
    }
    return (c->root->type == YAML_SEQUENCE_NODE);
}

bool yaml_config_is_scalar(yaml_config_t c)
{
    if (c == NULL || c->root == NULL) {
        return false;
    }
    return (c->root->type == YAML_SCALAR_NODE);
}

bool yaml_config_is_mapping(yaml_config_t c)
{
    if (c == NULL || c->root == NULL) {
        return false;
    }
    return (c->root->type == YAML_MAPPING_NODE);
}

bool yaml_config_mapping_next(yaml_config_t conf, yaml_config_iterator_t *it,
                              char **key, yaml_config_t *child)
{
    yaml_node_pair_t *pair = NULL;
    yaml_node_t *k = NULL, *v = NULL;
    yaml_config_t tmp = NULL;

    if (!yaml_config_is_mapping(conf)) {
        return false;
    }

    if ((*it) != NULL) {
        pair = (yaml_node_pair_t*)*it;
    } else {
        pair = conf->root->data.mapping.pairs.start;
    }

    if (pair >= conf->root->data.mapping.pairs.top) {
        return false;
    }

    k = yaml_document_get_node(conf->doc, pair->key);
    v = yaml_document_get_node(conf->doc, pair->value);

    if ((tmp = yaml_config_new()) == NULL) {
        return false;
    }

    tmp->doc = conf->doc;
    tmp->root = v;
    tmp->weak = true;

    *key = (char*)k->data.scalar.value;
    *child = tmp;

    ++pair;
    *it = pair;

    return true;
}
