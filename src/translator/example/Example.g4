grammar Example;

// Lexer -----

NormalLeftArrow: '->';
NormalRightArrow: '<-';
Comma: ':';
Less: '<';
More: '>';
Colon: ',';
EOL: '\r\n' | '\n';

fragment Digit: [0-9]+;
Number: Digit+;

Space: ' ' | '\t';
Other:.+?;

// Parser -----

program: (command (EOL command?)*)?;
command: messageCommand | dependencyCommand | EOL;

messageCommand: dependencyID? actorName arrow actorName Comma message;
dependencyCommand: actorName Comma (dependency (Colon dependency)*);

dependency: number (comparison number)+;
dependencyID: number ')';

message: text | (EOL Space text)+;

text: Space* textInternal Space*;
textInternal: ~Space (~EOL* ~Space)?;

number: Space* Number Space*;

comparison: Less | More;

arrow: NormalLeftArrow | NormalRightArrow;

actorName: text;

// vim: filetype=antlr
