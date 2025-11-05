grammar Java;

// Parser Rules
compilationUnit
    : packageDeclaration? importDeclaration* typeDeclaration* EOF
    ;

packageDeclaration
    : PACKAGE qualifiedName SEMI
    ;

importDeclaration
    : IMPORT STATIC? qualifiedName (DOT STAR)? SEMI
    ;

typeDeclaration
    : classDeclaration
    | interfaceDeclaration
    | enumDeclaration
    | annotationTypeDeclaration
    ;

classDeclaration
    : CLASS IDENTIFIER (EXTENDS type)? (IMPLEMENTS typeList)?
      classBody
    ;

interfaceDeclaration
    : INTERFACE IDENTIFIER (EXTENDS typeList)? interfaceBody
    ;

enumDeclaration
    : ENUM IDENTIFIER (IMPLEMENTS typeList)? enumBody
    ;

annotationTypeDeclaration
    : AT INTERFACE IDENTIFIER annotationTypeBody
    ;

classBody
    : LBRACE classBodyDeclaration* RBRACE
    ;

interfaceBody
    : LBRACE interfaceBodyDeclaration* RBRACE
    ;

enumBody
    : LBRACE enumConstant* (COMMA enumConstant)* (COMMA)? enumBodyDeclarations? RBRACE
    ;

annotationTypeBody
    : LBRACE annotationTypeElementDeclaration* RBRACE
    ;

classBodyDeclaration
    : SEMI
    | STATIC? block
    | memberDeclaration
    ;

memberDeclaration
    : methodDeclaration
    | fieldDeclaration
    | constructorDeclaration
    | genericMethodDeclaration
    | genericConstructorDeclaration
    | classDeclaration
    | interfaceDeclaration
    ;

methodDeclaration
    : type IDENTIFIER formalParameters (THROWS qualifiedNameList)?
      (methodBody | SEMI)
    ;

genericMethodDeclaration
    : typeParameters methodDeclaration
    ;

fieldDeclaration
    : type variableDeclarators SEMI
    ;

constructorDeclaration
    : IDENTIFIER formalParameters (THROWS qualifiedNameList)?
      constructorBody
    ;

genericConstructorDeclaration
    : typeParameters constructorDeclaration
    ;

interfaceBodyDeclaration
    : SEMI
    | modifier* interfaceMemberDeclaration
    ;

interfaceMemberDeclaration
    : constDeclaration
    | interfaceMethodDeclaration
    | genericInterfaceMethodDeclaration
    | interfaceDeclaration
    | annotationTypeDeclaration
    | classDeclaration
    | enumDeclaration
    ;

constDeclaration
    : type constantDeclarator SEMI
    ;

constantDeclarator
    : IDENTIFIER (LBRACK RBRACK)* ASSIGN variableInitializer
    ;

interfaceMethodDeclaration
    : (type | VOID) IDENTIFIER formalParameters (THROWS qualifiedNameList)? SEMI
    ;

genericInterfaceMethodDeclaration
    : typeParameters interfaceMethodDeclaration
    ;

variableDeclarators
    : variableDeclarator (COMMA variableDeclarator)*
    ;

variableDeclarator
    : variableDeclaratorId (ASSIGN variableInitializer)?
    ;

variableDeclaratorId
    : IDENTIFIER (LBRACK RBRACK)*
    ;

variableInitializer
    : arrayInitializer
    | expression
    ;

arrayInitializer
    : LBRACE (variableInitializer (COMMA variableInitializer)* (COMMA)? )? RBRACE
    ;

enumConstant
    : annotation* IDENTIFIER (LPAREN argumentList? RPAREN)? (classBody)?
    ;

enumBodyDeclarations
    : SEMI classBodyDeclaration*
    ;

methodBody
    : block
    ;

constructorBody
    : block
    ;

interfaceMethodModifier
    : annotation
    | PUBLIC
    | ABSTRACT
    | DEFAULT
    | STATIC
    | STRICTFP
    ;

modifier
    : annotation
    | PUBLIC
    | PROTECTED
    | PRIVATE
    | STATIC
    | ABSTRACT
    | FINAL
    | NATIVE
    | SYNCHRONIZED
    | TRANSIENT
    | VOLATILE
    | STRICTFP
    ;

qualifiedName
    : IDENTIFIER (DOT IDENTIFIER)*
    ;

qualifiedNameList
    : qualifiedName (COMMA qualifiedName)*
    ;

formalParameters
    : LPAREN formalParameterList? RPAREN
    ;

formalParameterList
    : formalParameter (COMMA formalParameter)* (COMMA lastFormalParameter)?
    ;

formalParameter
    : variableModifier* type variableDeclaratorId
    ;

lastFormalParameter
    : variableModifier* type ELLIPSIS variableDeclaratorId
    ;

variableModifier
    : FINAL
    | annotation
    ;

type
    : primitiveType (LBRACK RBRACK)*
    | classOrInterfaceType (LBRACK RBRACK)*
    | typeVariable (LBRACK RBRACK)*
    ;

classOrInterfaceType
    : (IDENTIFIER typeArguments? DOT)* IDENTIFIER typeArguments?
    ;

primitiveType
    : BOOLEAN
    | CHAR
    | BYTE
    | SHORT
    | INT
    | LONG
    | FLOAT
    | DOUBLE
    ;

typeVariable
    : IDENTIFIER typeArguments?
    | classOrInterfaceType
    ;

typeArguments
    : LT typeArgument (COMMA typeArgument)* GT
    ;

typeArgument
    : type
    | QUESTION (EXTENDS type | SUPER type)?
    ;

typeParameters
    : LT typeParameter (COMMA typeParameter)* GT
    ;

typeParameter
    : IDENTIFIER (EXTENDS typeBound)?
    ;

typeBound
    : type (BITAND type)*
    ;

block
    : LBRACE blockStatement* RBRACE
    ;

blockStatement
    : localVariableDeclarationStatement
    | statement
    | typeDeclaration
    ;

localVariableDeclarationStatement
    : localVariableDeclaration SEMI
    ;

localVariableDeclaration
    : variableModifier* type variableDeclarators
    ;

statement
    : block
    | ASSERT expression (COLON expression)? SEMI
    | IF parExpression statement (ELSE statement)?
    | FOR LPAREN forControl RPAREN statement
    | WHILE parExpression statement
    | DO statement WHILE parExpression SEMI
    | TRY block (catchClause+ finallyBlock? | finallyBlock?)
    | SWITCH parExpression LBRACE switchBlockStatementGroup* switchLabel* RBRACE
    | SYNCHRONIZED parExpression block
    | RETURN expression? SEMI
    | THROW expression SEMI
    | BREAK IDENTIFIER? SEMI
    | CONTINUE IDENTIFIER? SEMI
    | SEMI
    | statementExpression SEMI
    | IDENTIFIER COLON statement
    ;

switchBlockStatementGroup
    : switchLabel+ blockStatement+
    ;

switchLabel
    : CASE constantExpression COLON
    | DEFAULT COLON
    ;

forControl
    : enhancedForControl
    | forInit? SEMI expression? SEMI forUpdate?
    ;

forInit
    : localVariableDeclaration
    | statementExpressionList
    ;

enhancedForControl
    : variableModifier* type IDENTIFIER COLON expression
    ;

forUpdate
    : statementExpressionList
    ;

statementExpressionList
    : statementExpression (COMMA statementExpression)*
    ;

statementExpression
    : expression
    ;

constantExpression
    : expression
    ;

expression
    : primary
    | expression DOT IDENTIFIER (LPAREN expressionList? RPAREN)?
    | expression DOT THIS
    | expression DOT NEW nonWildcardTypeArguments? innerCreator
    | expression DOT SUPER LPAREN expressionList? RPAREN
    | expression DOT IDENTIFIER
    | expression LBRACK expression RBRACK
    | expression (INC | DEC)
    | (PLUS | MINUS) expression
    | (NOT | BNOT) expression
    | (INC | DEC) expression
    | expression (MUL | DIV | MOD) expression
    | expression (PLUS | MINUS) expression
    | expression (LT LT | GT GT GT | GT GT) expression
    | expression (LE | GE | LT | GT) expression
    | expression (EQUAL | NOTEQUAL) expression
    | expression BITAND expression
    | expression CARET expression
    | expression BITOR expression
    | expression AND expression
    | expression OR expression
    | expression QUESTION expression COLON expression
    | expression (ASSIGN | ADD_ASSIGN | SUB_ASSIGN | MUL_ASSIGN | DIV_ASSIGN | MOD_ASSIGN | AND_ASSIGN | OR_ASSIGN | XOR_ASSIGN | LSHIFT_ASSIGN | RSHIFT_ASSIGN | URSHIFT_ASSIGN) expression
    ;

primary
    : LPAREN expression RPAREN
    | THIS (LPAREN expressionList? RPAREN)?
    | SUPER (LPAREN expressionList? RPAREN)?
    | literal
    | NEW creator
    | IDENTIFIER (LPAREN expressionList? RPAREN)? (LBRACK expression RBRACK)* (DOT IDENTIFIER (LPAREN expressionList? RPAREN)?)*
    | type (LPAREN expressionList? RPAREN)? (LBRACK expression RBRACK)* (DOT IDENTIFIER (LPAREN expressionList? RPAREN)?)*
    | VOID (DOT CLASS)
    | nonWildcardTypeArguments (explicitGenericInvocationSuffix | THIS arguments)
    ;

creator
    : nonWildcardTypeArguments createdName classCreatorRest
    | createdName (nonWildcardTypeArguments classCreatorRest | arrayCreatorRest)
    ;

createdName
    : IDENTIFIER typeArguments? (DOT IDENTIFIER typeArguments?)*
    | primitiveType
    ;

innerCreator
    : IDENTIFIER nonWildcardTypeArguments? classCreatorRest
    ;

arrayCreatorRest
    : LBRACK (RBRACK (LBRACK RBRACK)* arrayInitializer | expression RBRACK (LBRACK expression RBRACK)* (LBRACK RBRACK)*)
    ;

classCreatorRest
    : arguments classBody?
    ;

explicitGenericInvocation
    : nonWildcardTypeArguments explicitGenericInvocationSuffix
    ;

nonWildcardTypeArguments
    : LT typeList GT
    ;

typeList
    : type (COMMA type)*
    ;

nonWildcardTypeArgumentsOrDiamond
    : LT GT
    | nonWildcardTypeArguments
    ;

explicitGenericInvocationSuffix
    : SUPER superSuffix
    | IDENTIFIER arguments
    ;

arguments
    : LPAREN expressionList? RPAREN
    ;

superSuffix
    : arguments
    | DOT IDENTIFIER arguments?
    ;

expressionList
    : expression (COMMA expression)*
    ;

parExpression
    : LPAREN expression RPAREN
    ;

catchClause
    : CATCH LPAREN catchType IDENTIFIER RPAREN block
    ;

catchType
    : qualifiedName (BITOR qualifiedName)*
    ;

finallyBlock
    : FINALLY block
    ;

literal
    : integerLiteral
    | floatingPointLiteral
    | characterLiteral
    | stringLiteral
    | booleanLiteral
    | nullLiteral
    ;

integerLiteral
    : DECIMAL_LITERAL
    | HEX_LITERAL
    | OCT_LITERAL
    | BINARY_LITERAL
    ;

floatingPointLiteral
    : FLOAT_LITERAL
    | HEX_FLOAT_LITERAL
    ;

characterLiteral
    : CHAR_LITERAL
    ;

stringLiteral
    : STRING_LITERAL
    ;

booleanLiteral
    : TRUE
    | FALSE
    ;

nullLiteral
    : NULL
    ;

// Lexer Rules
PACKAGE: 'package';
IMPORT: 'import';
CLASS: 'class';
INTERFACE: 'interface';
ENUM: 'enum';
AT: 'at';
EXTENDS: 'extends';
IMPLEMENTS: 'implements';
STATIC: 'static';
PUBLIC: 'public';
PROTECTED: 'protected';
PRIVATE: 'private';
ABSTRACT: 'abstract';
FINAL: 'final';
NATIVE: 'native';
SYNCHRONIZED: 'synchronized';
TRANSIENT: 'transient';
VOLATILE: 'volatile';
STRICTFP: 'strictfp';
VOID: 'void';
BOOLEAN: 'boolean';
CHAR: 'char';
BYTE: 'byte';
SHORT: 'short';
INT: 'int';
LONG: 'long';
FLOAT: 'float';
DOUBLE: 'double';
THROWS: 'throws';
THROW: 'throw';
TRY: 'try';
CATCH: 'catch';
FINALLY: 'finally';
SWITCH: 'switch';
CASE: 'case';
DEFAULT: 'default';
DO: 'do';
WHILE: 'while';
FOR: 'for';
IF: 'if';
ELSE: 'else';
BREAK: 'break';
CONTINUE: 'continue';
RETURN: 'return';
NEW: 'new';
ASSERT: 'assert';
THIS: 'this';
SUPER: 'super';
NULL: 'null';
TRUE: 'true';
FALSE: 'false';
INSTANCEOF: 'instanceof';
LPAREN: '(';
RPAREN: ')';
LBRACE: '{';
RBRACE: '}';
LBRACK: '[';
RBRACK: ']';
SEMI: ';';
COMMA: ',';
DOT: '.';
ASSIGN: '=';
GT: '>';
LT: '<';
NOT: '!';
BITNOT: '~';
QUESTION: '?';
COLON: ':';
EQUAL: '==';
LE: '<=';
GE: '>=';
NOTEQUAL: '!=';
AND: '&&';
OR: '||';
INC: '++';
DEC: '--';
ADD: '+';
SUB: '-';
MUL: '*';
DIV: '/';
BITAND: '&';
BITOR: '|';
CARET: '^';
MOD: '%';
ADD_ASSIGN: '+=';
SUB_ASSIGN: '-=';
MUL_ASSIGN: '*=';
DIV_ASSIGN: '/=';
AND_ASSIGN: '&=';
OR_ASSIGN: '|=';
XOR_ASSIGN: '^=';
MOD_ASSIGN: '%=';
LSHIFT_ASSIGN: '<<=';
RSHIFT_ASSIGN: '>>=';
URSHIFT_ASSIGN: '>>>=';
ELLIPSIS: '...';
BITAND_ASSIGN: '&=';
ARROW: '->';
COLONCOLON: '::';

IDENTIFIER: [a-zA-Z_][a-zA-Z0-9_]*;

DECIMAL_LITERAL: [0-9]+;
HEX_LITERAL: '0' [xX] [0-9a-fA-F]+;
OCT_LITERAL: '0' [0-7]+;
BINARY_LITERAL: '0' [bB] [01]+;

FLOAT_LITERAL: [0-9]+ '.' [0-9]* | '.' [0-9]+ | [0-9]+ [fFdD];
HEX_FLOAT_LITERAL: '0' [xX] [0-9a-fA-F]+ '.'? [pP] [+-]? [0-9]+ [fFdD]?;

CHAR_LITERAL: '\'' (~['\\\r\n] | EscapeSequence) '\'';
STRING_LITERAL: '"' (~["\\\r\n] | EscapeSequence)* '"';

fragment
EscapeSequence
    : '\\' [btnfr"'\\]
    | '\\' [0-7] [0-7]?
    | '\\' 'u' [0-9a-fA-F] [0-9a-fA-F] [0-9a-fA-F] [0-9a-fA-F]
    ;

LINE_COMMENT: '//' ~[\r\n]* -> skip;
BLOCK_COMMENT: '/*' .*? '*/' -> skip;
WS: [ \t\r\n\u000C]+ -> skip;
