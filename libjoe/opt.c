#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// for getopt_long used
#include <stddef.h>
#include <getopt.h>

#include "misc.h"
#include "opt.h"

static simple_list *head = NULL;

static int opt_len(void)
{
    if (!head) { return 0; }

    int count = 0;

    simple_list *list = head;

    while(list) {
        count++;
        list = list->next;
    }

    return count;
}

int opt_reg(struct option opt, opt_func func, char *desc)
{
    if (!opt.name) {
        PRINT_ERR("%s() invalid args.\n", __func__);
        return -1;
    }

    simple_list *node = (simple_list *)malloc(sizeof(simple_list));
    if (!node) {
        PRINT_ERR("%s() malloc failed : %s\n", __func__, strerror(errno));
        return -1;
    }
    memset(node, 0, sizeof(simple_list));

    memcpy(&(node->opt), &opt, sizeof(struct option));
    node->desc = desc;
    node->func = func;

    if (!head) { head = node; }
    else {
        simple_list *last = head;
        while(last->next) { last = last->next; }
        last->next = node;
    }

    return 0;
}

int opt_usage(char *filename)
{
    if (filename) {
        PRINT_RAW("\nUsage : %s [operation] <args>\n", filename);
    }

    if (!head) {
        return 0;
    }

    PRINT_RAW("[ operation ]\n");
    simple_list *node = head;
    while(node) {
        if (!node->opt.name) { continue; }

        PRINT_RAW(" --%s\t : ", node->opt.name);
        if (node->desc) { PRINT_RAW("%s", node->desc); }
        PRINT_RAW("\n");
        
        node = node->next;
    }
    PRINT_RAW("\n");

    return 0;
}

int opt_free(void)
{
    if (!head) { return 0; }

    simple_list *node = head;

    while(node) {
        simple_list *next = node->next;
        free(node);
        node = next;
    }

    return 0;
}

int opt_run(int argc, char *argv[])
{
    if (!head) { return 0; }

    struct option *opt_ls = (struct option *)malloc(sizeof(struct option) * opt_len());
    if (!opt_ls) {
        PRINT_ERR("%s malloc failed : %s\n", __func__, strerror(errno));
        return -1;
    }

    // copy simple_list opt to opt_ls
    simple_list *copy = head;
    struct option *dest = opt_ls;
    while(copy)
    {
        memcpy(dest++, &copy->opt, sizeof(struct option));
        copy = copy->next;
    }

    int opt = 0;
    int opt_idx = 0;
    while ((opt = getopt_long(argc, argv, "", opt_ls, &opt_idx)) != -1)
    {
        // something wrong ... ( maybe have not argument )
        if (opt) { continue; }

        simple_list *node = head;
        while(node) {
            if (!strncmp(opt_ls[opt_idx].name, node->opt.name, sizeof(node->opt.name))) {

                if (node->func) {
                    if (node->func(optarg)) {
                        // do something ...
                    }
                }
                break;
            }
            node = node->next;
        }
    }

    free(opt_ls);

    return 0;
}
