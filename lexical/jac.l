

%{
	int flag = 0;
	int line = 1 , col = 1, begin_l, begin_col;

	void print_error(int error_case);
	void printToken(char * token);
	void printID(char * ptr);
	void prepareString();
	void buildString();
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
char			[a-zA-Z]
symbol		[$_]
signal		[+-]
declit 			"0"|[1-9](("_"|{number})*{number})?
declit_zero {number}(("_"|{number})*{number})?
allowed		[ \t\f]
newline		\n
form_feed	\f
carriage_return	\r
crlf	({carriage_return}{newline})
escape_chars		[\"fnrt\\]
invalid_escape \\[^{escape_chars}]
line_change	({newline}|{carriage_return}|{crlf})
aspa	\"


%X STRING COMMENT LINE_COMMENT STRING_ERROR

%%

"boolean"							{printToken("BOOL");}
"true"|"false" 				{printID("BOOLLIT");}
"class"								{printToken("CLASS");}
"do"									{printToken("DO");}
".length"							{printToken("DOTLENGTH");}
"double"							{printToken("DOUBLE");}
"else"								{printToken("ELSE");}
"if"									{printToken("IF");}
"int"									{printToken("INT");}
"Integer.parseInt"		{printToken("PARSEINT");}
"System.out.println"	{printToken("PRINT");}
"public"							{printToken("PUBLIC");}
"return"							{printToken("RETURN");}
"static"							{printToken("STATIC");}
"String"							{printToken("STRING");}
"void"								{printToken("VOID");}
"while"								{printToken("WHILE");}
"("										{printToken("OCURV");}
")"										{printToken("CCURV");}
"{"										{printToken("OBRACE");}
"}"										{printToken("CBRACE");}
"["										{printToken("OSQUARE");}
"]"										{printToken("CSQUARE");}
"&&"									{printToken("AND");}
"||"									{printToken("OR");}
"<"										{printToken("LT");}
">"										{printToken("GT");}
"=="									{printToken("EQ");}
"!="									{printToken("NEQ");}
"<="									{printToken("LEQ");}
">="									{printToken("GEQ");}
"+"										{printToken("PLUS");}
"-"										{printToken("MINUS");}
"*"										{printToken("STAR");}
"/"										{printToken("DIV");}
"%"										{printToken("MOD");}
"!"										{printToken("NOT");}
"="										{printToken("ASSIGN");}
";"										{printToken("SEMI");}
","										{printToken("COMMA");}
{reserved_words}			{printID("RESERVED");}

{declit}																													{printID("DECLIT");}

({char}|{symbol})({number}|{char}|{symbol})*											{printID("ID");}

{declit_zero}?"."({declit_zero}((e|E)(("+"|"-")?){declit_zero})?)	{printID("REALLIT");}
{declit_zero}"."({declit_zero}((e|E)(("+"|"-")?){declit_zero})?)?	{printID("REALLIT");}
{declit_zero}(".")?((e|E)("+"|"-")?{declit_zero})									{printID("REALLIT");}


"//"															{BEGIN LINE_COMMENT; col+=2;}
<LINE_COMMENT>{line_change}				{BEGIN 0; line++; col=1;}
<LINE_COMMENT>.										{col+=yyleng;}


"/*"														{BEGIN COMMENT; begin_l = line; begin_col = col; col+=yyleng;}
<COMMENT><<EOF>>								{print_error(3); BEGIN 0;}
<COMMENT>"*/"										{col+=yyleng; BEGIN 0;}
<COMMENT>{line_change}					{line++; col=1;}
<COMMENT>.											{col+=yyleng;}


{aspa}([^\\\"\n\r\r\n]|(\\{escape_chars}))*{aspa}		{if(flag) printf("STRLIT(%s)\n", yytext); col+=yyleng;}
{aspa}																								{BEGIN STRING_ERROR; begin_l = line; begin_col = col; col+=yyleng;}

<STRING_ERROR>\"													{col+=yyleng; BEGIN 0;}
<STRING_ERROR>([\n\r\r\n]|<EOF>)					{print_error(4); line++; col = 1;BEGIN 0;}
<STRING_ERROR>\\[\"fnrt\\]								{col+=yyleng;}
<STRING_ERROR>\\([^\"fnrt\\\n\r\r\n])?				{print_error(2); col+=yyleng;}
<STRING_ERROR>.														{col+=yyleng;}

{line_change}										{line++; col = 1;}
{allowed}											{col+=yyleng;}
.													{print_error(1); col+=yyleng;}



%%

int main(int argc, char * argv[]){
	if(argc == 2){
		if(!strcmp(argv[1],"-l")){ /*|| !strcmp(argv[1],"-1")*/
			flag = 1;
		}
	}
	yylex();

	return 0;
}

int yywrap(){
	return 1;
}


void printToken(char * token){

	col+=yyleng;

	if(flag)
		printf("%s\n" , token);

}


void printID(char * ptr){

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
