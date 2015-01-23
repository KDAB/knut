#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

extern FILE *yyin;
extern int yylex(void);
extern int yyparse(void);

int linenum;

static const char *INFILE = NULL;

static void
panic(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);

    fprintf(stderr, "PANIC: ");

    vfprintf(stderr, fmt, args);

    putchar('\n');

    va_end(args);

    exit(EXIT_FAILURE);
}

void
yyerror(const char *s)
{
    panic("%s:%d: %s\n", INFILE, linenum, s);
}

int main(int argc, char *argv[])
{
    FILE *rcfile;

    INFILE = argv[1];

    if ((rcfile = fopen(INFILE, "r")) == NULL) {
        panic("cannot open %s: %s", INFILE, strerror(errno));
    }


    linenum = 0;

    yyin = rcfile;

    do {
        yyparse();
    } while (!feof(yyin));

    fclose(rcfile);

    return 0;
}
