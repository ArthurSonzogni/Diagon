grammar PlanarGraph;

graph: edges (EOL edges)* EOF;
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

// Ignored TOKENS
WS: [ \t] -> skip;
COMMENT: '/*' .*? '*/' -> skip;
LINE_COMMENT: '//' .*? '\r'? '\n' -> skip;

// vim: filetype=antlr
