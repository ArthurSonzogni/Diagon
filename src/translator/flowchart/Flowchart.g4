grammar Flowchart;

// Ignored TOKENS
WS: [ \t\n] -> skip;
COMMENT: '/*' .*? '*/' -> skip;
LINE_COMMENT: '//' .*? '\r'? '\n' -> skip;

// Program
program: instruction*;

instruction: noop
           | element
           | condition
           | group
           | whileloop
           | doloop
           | returninstruction
           | switchinstruction
           ;

noop: 'noop' ';'?;

element: string ';'?;

string: STRING_SIMPLE_QUOTE
      | STRING_DOUBLE_QUOTE
      ;
STRING_SIMPLE_QUOTE: '\'' ( '\\\'' | . )*? '\'';
STRING_DOUBLE_QUOTE: '"' ( '\\"' | . )*? '"';

condition: 'if' '(' string ')' instruction ( 'else' instruction )?;

whileloop: 'while' '(' string ')' instruction;

doloop: 'do' instruction 'while' '(' string ')' ';'?;

group: '{' program '}';

returninstruction: 'return' instruction;

switchinstruction:
  'switch' '(' string ')' '{'
    ( 'case' string ':' instruction)*
  '}'
  ;

// vim: filetype=antlr
