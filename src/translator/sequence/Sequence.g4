grammar Sequence;

// Lexer -----

NormalRightArrow: '->';
NormalLeftArrow: '<-';
Comma: ':';
Less: '<';
More: '>';
Colon: ',';
EOL: '\r\n' | '\n';

fragment Digit: [0-9]+;
Number: Digit+;

Space: ' ' | '\t';
Other: .;

// Parser -----

program: (command? (EOL command?)*)  EOF;
command: messageCommand | dependencyCommand;

messageCommand: dependencyID? text arrow text Comma text;
dependencyCommand: text Comma dependencies;

dependency: number (comparison number)+;
dependencyID: number ')';
dependencies: (dependency (Colon dependency)*)?;

text: Space* textInternal Space*;
textInternal: ~(Space|EOL) (~EOL* ~(Space|EOL))?;

number: Space* Number Space*;

comparison: Less | More;

arrow: NormalLeftArrow | NormalRightArrow;
// vim: filetype=antlr
