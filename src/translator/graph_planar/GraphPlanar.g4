grammar GraphPlanar;

// Ignored TOKENS
WS: [ \t]+ -> channel(HIDDEN);
COMMENT: '/*' .*? '*/' -> channel(HIDDEN);
LINE_COMMENT: '//' .*? ('\r'? '\n' | EOF) -> channel(HIDDEN);

graph: EOL* edges (EOL+ edges)* EOL* EOF;
edges: node (arrow node)+;
arrow: RIGHT_ARROW | NONE_ARROW | LEFT_RIGHT_ARROW | LEFT_ARROW;
node: ID | STRING;

RIGHT_ARROW: '->';
NONE_ARROW: '--';
LEFT_RIGHT_ARROW: '<->';
LEFT_ARROW: '<-';

STRING: '"' ( '\\"' | . )*? '"';
ID: LETTER+;
fragment LETTER: ~[ \t\n\r\-<>"];

EOL: '\r\n' | '\n';

// vim: filetype=antlr
