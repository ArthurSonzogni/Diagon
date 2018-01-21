parser grammar SequenceParser;

options {
	tokenVocab = SequenceLexer;
}

program: command (EOL command)* EOF;
command: message | actor | /* empty */;

message: messageID? Words arrow Words Colon messageText;
actor: Words Colon (Number (Comma Number)*);

arrow: NormalLeftArrow
     | DashedLeftArrow
     | DottedLeftArrow
     | NormalRightArrow
     | DashedRightArrow
     | DottedRightArrow;

messageText: Words        # singleLineText
           | (EOL Words)+ # multiLineText;

messageID: Number CloseParenthesis;

// vim: filetype=antlr
