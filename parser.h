/* depends on lexer.h */
/* depends on target.h */

#ifdef MBL_PARSER_H
#error multiple inclusion
#endif
#define MBL_PARSER_H

void parse_file(LexState * ls, EmitState * es);

