lexer grammar SequenceLexer;

// Fragments.
fragment Digit: [0-9]+;
fragment Letter: ~[<:>\-0-9\r\n) \t];
fragment Word: Letter ( Letter | Digit )*;
fragment EscapedString: '"' (~'"')* '"';

// The Arrows.
NormalLeftArrow: '<-';
NormalRightArrow: '->';

Words: Word (Space+ Word)* | EscapedString;
Number: Digit+;

Colon: ':';
Less: '<';
CloseParenthesis: ')';

Space:  (' ' | '\t') {skip();} ;
EOL: '\n' | '\r\n';

// vim: filetype=antlr
