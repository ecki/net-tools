/* Tolerant /proc file parser. Copyright 1998 Andi Kleen */ 

#include <string.h> 
#include <stdarg.h>
#include <stdio.h>

/* Caller must free return string. */ 
char * 
proc_gen_fmt(char *name, FILE *fh, ...)
{
	char buf[512], format[512] = ""; 
	char *title, *head; 
	va_list ap;

	if (!fgets(buf, sizeof buf, fh)) 
		return NULL; 

	va_start(ap,fh); 
	head = strtok(buf, " \t"); 
	title = va_arg(ap, char *); 
	while (title && head) { 
		if (!strcmp(title, head)) { 
			strcat(format, va_arg(ap, char *)); 
			title = va_arg(ap, char *); 
		} else {
			strcat(format, "%*[^ \t]"); 
		}
		strcat(format, " "); 
		head = strtok(NULL, " \t"); 
	}
	va_end(ap); 

	if (title) { 
		fprintf(stderr, "warning: %s does not contain required field %s\n",
						name, title); 
		return NULL; 
	}
	return strdup(format); 
}
