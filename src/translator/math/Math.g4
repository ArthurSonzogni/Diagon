/*
  BSD License

  Copyright (c) 2013, Tom Everett
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  3. Neither the name of Tom Everett nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
  BSD License (Same license)

  Copyright (c) 2018 Arthur Sonzogni
  This file is a modified version taken from:
  https://github.com/antlr/grammars-v4e/blob/master/arithmetic/arithmetic.g4
*/

grammar Math;

multilineEquation : equation (newlines equation)* EOL? EOF ;
newlines : EOL+;
equation : expression (relop expression)* ;
expression : term (addop term)* ;
term : factor (mulop factor)* ;
factor : valueBang (powop valueBang)* ;
valueBang: value | valueBang BANG;
value : (PLUS | MINUS)? atom ;
atom : scientific | variable | function | matrix | LPAREN expression RPAREN ;
scientific : SCIENTIFIC_NUMBER ;
function : variable LPAREN equation (',' equation)* RPAREN ;
variable : VARIABLE ;
matrix : LBRACKET matrixLine (';' matrixLine)* RBRACKET;
matrixLine : expression ( ',' expression ) *;
relop : EQ | GT | LT | GE | LE;
addop : PLUS | MINUS ;
mulop : TIMES | DIV ;
powop : POW | SUBSCRIPT ;
VARIABLE : VALID_ID_START VALID_ID_CHAR* | '...' ;

fragment VALID_ID_START : ('a' .. 'z') | ('A' .. 'Z') ;
fragment VALID_ID_CHAR : VALID_ID_START | ('0' .. '9') ;
SCIENTIFIC_NUMBER : NUMBER (E SIGN? NUMBER)? ;

//The integer part gets its potential sign from the value rule

fragment NUMBER : ('0' .. '9') + ('.' ('0' .. '9') +)?  ;
fragment E : 'E' | 'e' ;
fragment SIGN : ('+' | '-') ;

LPAREN : '(' ;
RPAREN : ')' ;
LBRACKET : '[' ;
RBRACKET : ']' ;
PLUS : '+' ;
MINUS : '-' ;
TIMES : '*' ;
BANG: '!';
DIV : '/' ;
GT : '>' ; 
LT : '<' ; 
GE : '>=' ; 
LE : '<=' ; 
EQ : '=' ; 
POINT : '.' ; 
POW : '^' ;
SUBSCRIPT: '_' ;
EOL : '\r\n' | '\n' ;
WS : [ \t] + -> skip ; 

// vim: filetype=antlr
