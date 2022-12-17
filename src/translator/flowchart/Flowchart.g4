grammar Flowchart;

// Ignored TOKENS
WS: [ \t\n]+ -> channel(HIDDEN);
COMMENT: '/*' .*? '*/' -> channel(HIDDEN);
LINE_COMMENT: '//' .*? ('\r'? '\n' | EOF) -> channel(HIDDEN);

SEMICOLON: ';';
PL: '(';
PR: ')';
BL: '{';
BR: '}';
DO: 'do';
ELSE: 'else';
IF: 'if';
NOOP: 'noop';
RETURN: 'return';
SWITCH: 'switch';
WHILE: 'while';

STRING_SIMPLE_QUOTE: '\'' ( '\\\'' | . )*? '\'';
STRING_DOUBLE_QUOTE: '"' ( '\\"' | . )*? '"';

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

noop: NOOP SEMICOLON?;

element: string SEMICOLON?;

string: STRING_SIMPLE_QUOTE
      | STRING_DOUBLE_QUOTE
      ;

condition: IF PL string PR instruction ( ELSE instruction )?;

whileloop: WHILE PL string PR instruction;

doloop: DO instruction WHILE PL string PR SEMICOLON?;

group: BL program BR;

returninstruction: RETURN instruction;

switchinstruction:
  SWITCH PL string PR BL
    ( CASE string ':' instruction)*
  BR
  ;

// vim: filetype=antlr
