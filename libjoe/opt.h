#ifndef _MISC_OPTION_H
#define _MISC_OPTION_H

#include <stddef.h>

#include <getopt.h>
/* define in getopt.h
struct option {
    const char *name;
    int        has_arg;   // no_argument or required_argument
    int        *flag;
    int        val;
};
 */

typedef int (*opt_func)(char *);

typedef struct _simple_list {

    struct option opt;
    char *desc;
    opt_func func;

    struct _simple_list *next;

} simple_list;

int opt_reg(struct option opt, opt_func func, char *desc);
int opt_usage(char *filename);
int opt_free(void);
int opt_run(int argc, char *argv[]);

#endif
