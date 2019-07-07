#include <stdio.h>
#include <setjmp.h>
#include <ctype.h>

#include "lexer.h"

void error(LexState * ls, char const * msg)
{
	ls->bad = 1;
	fprintf(stderr, "%s\n", msg);
	longjmp(*ls->recovery, 1);
}

void advance(LexState * ls)
{
	while (isspace(ls->nextChar)) {
		ls->nextChar = fgetc(ls->file);
	}
	char c = ls->nextChar;
	if (isdigit(c)) {
		ls->tokenInteger = 0;
		do {
			ls->tokenInteger = ls->tokenInteger * 10 + (ls->nextChar - '0');
			ls->nextChar = fgetc(ls->file);
		} while (isdigit(ls->nextChar));
		ls->token = TK_INTEGER;
	} else if (c == '+') {
		ls->nextChar = fgetc(ls->file);
		ls->token = TK_PLUS;
	} else if (c == '-') {
		ls->nextChar = fgetc(ls->file);
		ls->token = TK_MINUS;
	} else if (c == '*') {
		ls->nextChar = fgetc(ls->file);
		ls->token = TK_ASTERISK;
	} else if (c == '/') {
		ls->nextChar = fgetc(ls->file);
		ls->token = TK_SLASH;
	} else if (c == '%') {
		ls->nextChar = fgetc(ls->file);
		ls->token = TK_PERCENT;
	} else if (c == '(') {
		ls->nextChar = fgetc(ls->file);
		ls->token = TK_LEFT_PAREN;
	} else if (c == ')') {
		ls->nextChar = fgetc(ls->file);
		ls->token = TK_RIGHT_PAREN;
	} else if (c == EOF) {
		/* Intentionally don't update nextChar */
		ls->token = TK_END_OF_FILE;
	} else {
		error(ls, "invalid character");
	}
}

void expect(LexState * ls, TokenType type)
{
	if (ls->token != type) {
		error(ls, "unexpected token");
	}
}

