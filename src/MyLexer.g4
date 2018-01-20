lexer grammar MyLexer;


Space:  (' ' | '\t') {skip();} ;

Int: Digit+;
Digit: [0-9];
Plus: '+';
Minus: '-';
Multiply: '*';
Divide: '/';
OpenParenthesis: '(';
CloseParenthesis: ')';

// vim: filetype=antlr
