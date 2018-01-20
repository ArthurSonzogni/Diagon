parser grammar MyParser;

options {
	tokenVocab = MyLexer;
}

expression:minusExpression EOF;

minusExpression: plusExpression (Minus plusExpression)*;
plusExpression: divideExpression (Plus divideExpression)*;
divideExpression: multiplyExpression (Divide multiplyExpression)*;
multiplyExpression: valueExpression (Multiply valueExpression)*;
valueExpression: Int | OpenParenthesis minusExpression CloseParenthesis;

// vim: filetype=antlr
