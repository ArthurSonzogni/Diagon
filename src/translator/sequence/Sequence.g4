grammar Sequence;

// Lexer -----

// Ignored TOKENS
WS: [\t]+ -> channel(HIDDEN);
COMMENT: '/' '*' .*? '*' '/' -> channel(HIDDEN);
LINE_COMMENT: '/' '/' (~('\n'))* -> channel(HIDDEN);

ARROW_RIGHT: '->';
ARROW_LEFT: '<-';
COMMA: ':';
LOWER: '<';
GREATER: '>';
COLON: ',';
EOL: '\r\n' | '\n';

NUMBER: [0-9]+;

SPACE: ' ' | '\t';
OTHER: .;

// Parser -----

program: (command? (EOL command?)*)  EOF;
command: messageCommand | dependencyCommand;

messageCommand: dependencyID? text arrow text COMMA text;
dependencyCommand: text COMMA dependencies;

dependency: number (comparison number)+;
dependencyID: number ')';
dependencies: (dependency (COLON dependency)*)?;

text: SPACE* textInternal SPACE*;
textInternal: ~(SPACE|EOL) (~EOL* ~(SPACE|EOL))?;

number: SPACE* NUMBER SPACE*;

comparison: LOWER | GREATER;

arrow: ARROW_LEFT | ARROW_RIGHT;
// vim: filetype=ANTLR
