lexer grammar SequenceLexer;

Space:  (' ' | '\t') {skip();} ;
EOL: '\n' | '\r\n';

// Fragments
fragment Digit: [0-9]+;
fragment Letter: [a-z] | [A-Z];
fragment Word: Letter ( Letter | Digit )*;

// The Arrow with different styles
NormalLeftArrow: '<-';
DashedLeftArrow: '<--';
DottedLeftArrow: '<..';
NormalRightArrow: '->';
DashedRightArrow: '-->';
DottedRightArrow: '..>';

Words: Word (Space+ Word)*;
Number: Digit+;

Colon: ':';
Comma: ',';
CloseParenthesis: ')';

// vim: filetype=antlr
