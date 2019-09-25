This was an academic project for the Compilers Course of the Informatics Engineering BSc in the University of Coimbra, year 2016/2017.

### Compiler code for the Java language, divided in four steps:

1. **Lexical Analyser** - Implemented in C using Lex. In this step tokens are assigned to each keyword in the code.
2. **Syntax Analyser** - Implemented in C using lex and yacc. In this step a Abstract Syntax Tree (AST) is generated based on the implemented grammar.
3. **Semantic Analyser** - Implemented in C. In this step, symbol tables are created and the AST is annotated. Detects lexical, syntax and semantic errors. 
4. **Intermediate Code Generation** - In this last step, the code with no errors is converted to intermediate code (LLVM).