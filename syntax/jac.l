%{
	#include "y.tab.h"
        #include "ast.h"
	extern int flag;
        extern int token_flag;
	int line = 1 , col = 1, begin_l, begin_col, actual_line=1, actual_col=1;

	void print_error(int error_case);
	void printToken(char * token);
	void printID(char * ptr);
%}

reserved1	("abstract"|"continue"|"for"|"new"|"switch"|"assert")
reserved2	("default"|"goto"|"package"|"synchronized"|"private"|"this")
reserved3	("break"|"implements"|"protected"|"throw"|"byte")
reserved4	("import"|"throws"|"case"|"enum"|"instanceof"|"transient")
reserved5	("catch"|"extends"|"short"|"try"|"char"|"final"|"interface")
reserved6	("finally"|"long"|"strictfp"|"volatile"|"const"|"float"|"native")
reserved7	("++"|"--"|"null"|"System"|"Integer"|"super")

reserved_words	({reserved1}|{reserved2}|{reserved3}|{reserved4}|{reserved5}|{reserved6}|{reserved7})

number		[0-9]
char		[a-zA-Z]
symbol		[$_]
signal		[+-]
declit 		"0"|[1-9](("_"|{number})*{number})?
declit_zero     {number}(("_"|{number})*{number})?
allowed		[ \t\f]
newline		\n
form_feed	\f
carriage_return	\r
crlf	        ({carriage_return}{newline})
escape_chars	[\"fnrt\\]
invalid_escape  \\[^{escape_chars}]
line_change	({newline}|{carriage_return}|{crlf})
aspa	        \"

%X COMMENT LINE_COMMENT STRING_ERROR

%%

"boolean"				{printToken("BOOL");if(token_flag) return BOOL;}
"true"|"false" 				{printID("BOOLLIT"); if(token_flag){yylval.strlit = strdup(yytext); return BOOLLIT;}}
"class"					{printToken("CLASS"); if(token_flag)return CLASS;}
"do"					{printToken("DO"); if(token_flag)return DO;}
".length"				{printToken("DOTLENGTH"); if(token_flag)return DOTLENGTH;}
"double"				{printToken("DOUBLE"); if(token_flag)return DOUBLE;}
"else"					{printToken("ELSE"); if(token_flag)return ELSE;}
"if"					{printToken("IF"); if(token_flag)return IF;}
"int"					{printToken("INT"); if(token_flag)return INT;}
"Integer.parseInt"		        {printToken("PARSEINT"); if(token_flag)return PARSEINT;}
"System.out.println"	                {printToken("PRINT"); if(token_flag)return PRINT;}
"public"				{printToken("PUBLIC"); if(token_flag)return PUBLIC;}
"return"				{printToken("RETURN"); if(token_flag)return RETURN;}
"static"				{printToken("STATIC"); if(token_flag)return STATIC;}
"String"				{printToken("STRING"); if(token_flag)return STRING;}
"void"					{printToken("VOID"); if(token_flag)return VOID;}
"while"					{printToken("WHILE"); if(token_flag)return WHILE;}
"("					{printToken("OCURV"); if(token_flag)return OCURV;}
")"					{printToken("CCURV"); if(token_flag)return CCURV;}
"{"				        {printToken("OBRACE"); if(token_flag)return OBRACE;}
"}"					{printToken("CBRACE"); if(token_flag)return CBRACE;}
"["					{printToken("OSQUARE"); if(token_flag)return OSQUARE;}
"]"					{printToken("CSQUARE"); if(token_flag)return CSQUARE;}
"&&"					{printToken("AND"); if(token_flag)return AND;}
"||"					{printToken("OR"); if(token_flag)return OR;}
"<"					{printToken("LT"); if(token_flag)return LT;}
">"				        {printToken("GT"); if(token_flag)return GT;}
"=="					{printToken("EQ"); if(token_flag)return EQ;}
"!="				        {printToken("NEQ"); if(token_flag)return NEQ;}
"<="					{printToken("LEQ"); if(token_flag)return LEQ;}
">="					{printToken("GEQ"); if(token_flag)return GEQ;}
"+"					{printToken("PLUS"); if(token_flag)return PLUS;}
"-"					{printToken("MINUS"); if(token_flag)return MINUS;}
"*"					{printToken("STAR"); if(token_flag)return STAR;}
"/"					{printToken("DIV"); if(token_flag)return DIV;}
"%"					{printToken("MOD"); if(token_flag)return MOD;}
"!"				        {printToken("NOT"); if(token_flag)return NOT;}
"="					{printToken("ASSIGN"); if(token_flag)return ASSIGN;}
";"					{printToken("SEMI"); if(token_flag)return SEMI;}
","					{printToken("COMMA"); if(token_flag)return COMMA;}
{reserved_words}			{printID("RESERVED"); if(token_flag)return RESERVED;}

{declit}		                {printID("DECLIT"); if(token_flag) {yylval.strlit = strdup(yytext); return DECLIT;}}

({char}|{symbol})({number}|{char}|{symbol})*	{printID("ID"); if(token_flag){yylval.strlit=(char*)strdup(yytext);return ID;}}

{declit_zero}?"."({declit_zero}((e|E)(("+"|"-")?){declit_zero})?)	{printID("REALLIT"); if(token_flag){yylval.strlit = strdup(yytext); return REALLIT;}}
{declit_zero}"."({declit_zero}((e|E)(("+"|"-")?){declit_zero})?)?	{printID("REALLIT"); if(token_flag){yylval.strlit = strdup(yytext); return REALLIT;}}
{declit_zero}(".")?((e|E)("+"|"-")?{declit_zero})			{printID("REALLIT"); if(token_flag){yylval.strlit = strdup(yytext); return REALLIT;}}

"//"					{BEGIN LINE_COMMENT; actual_col = col; col+=2;}
<LINE_COMMENT>{line_change}		{BEGIN 0; line++; actual_line = line; actual_col = col; col=1;}
<LINE_COMMENT>.				{actual_col = col; col+=yyleng;}

"/*"					{BEGIN COMMENT; begin_l = line; actual_col = col; begin_col = col; col+=yyleng;}
<COMMENT><<EOF>>			{print_error(3); BEGIN 0;}
<COMMENT>"*/"				{actual_col = col;  col+=yyleng; BEGIN 0;}
<COMMENT>{line_change}			{line++; actual_line = line;actual_col = col; col=1;}
<COMMENT>.				{actual_col = col; col+=yyleng;}

{aspa}([^\\\"\n\r\r\n]|(\\{escape_chars}))*{aspa}       {actual_col = col; printID("STRLIT"); if(token_flag){yylval.strlit = strdup(yytext); return STRLIT;}}
{aspa}  				{BEGIN STRING_ERROR; begin_l = line; begin_col = col; actual_col = col; col+=yyleng;}

<STRING_ERROR>\"			{actual_col = col; col+=yyleng; BEGIN 0;}
<STRING_ERROR>([\n\r\r\n]|<EOF>)	{actual_col = col;  print_error(4); line++; actual_line = line; col = 1;BEGIN 0;}
<STRING_ERROR>\\[\"fnrt\\]		{actual_col = col; col+=yyleng;}
<STRING_ERROR>\\([^\"fnrt\\\n\r\r\n])?	{print_error(2); actual_col = col; col+=yyleng;}
<STRING_ERROR>.				{actual_col = col;  col+=yyleng;}
<<EOF>>					{actual_col= col; return 0;}
{line_change}				{line++; actual_line = line; actual_col = 1; col = 1;}
{allowed}				{actual_col = col; col+=yyleng;}

.					{print_error(1); actual_col = col; col+=yyleng;}

%%

int yywrap(){
	return 1;
}

void printToken(char * token){
	actual_col = col;
	col+=yyleng;
	if(flag)
		printf("%s\n" , token);
}

void printID(char * ptr){
	actual_col = col;
	col += yyleng;
	if(flag)
		printf("%s(%s)\n", ptr, yytext);
}

void print_error(int error_case) {
	switch(error_case) {
		case 1://illegal char
			printf("Line %d, col %d: illegal character (%c)\n", line, col, *yytext);
			break;
		case 2://invalid sequence
			printf("Line %d, col %d: invalid escape sequence (%s)\n", line, col, yytext);
			break;
		case 3://untermidnasnd comment
			printf("Line %d, col %d: unterminated comment\n", begin_l, begin_col);
			break;
		case 4:
			printf("Line %d, col %d: unterminated string literal\n", begin_l, begin_col);
			break;
	}
}
