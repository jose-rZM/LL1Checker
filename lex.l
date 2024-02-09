%{
	#include<stdio.h>
%}
%%
\$	{ return 1; }
and	{ return 2; }
or	{ return 3; }
not	{ return 4; }
[a-zA-Z_][a-zA-Z_0-9]*	{ return 5; }
\(	{ return 6; }
\)	{ return 7; }
[ \t\r\n]	{}
.	{ return -1; }
%%
int set_yyin(const char* filename) {
	FILE* input = fopen(filename, "r");
	yyrestart(input);
	return 1;
}
int yywrap() {
	return 1;
}
