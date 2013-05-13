/*
 * prototypes for proc.c
 */
char *proc_gen_fmt(char *name, int more, FILE * fh,...);
int   proc_guess_fmt(char *name, FILE* fh,...);
FILE *proc_fopen(const char *name);

