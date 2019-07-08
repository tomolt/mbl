#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <ctype.h>

#include "lexer.h"

void error(LexState * ls, char const * msg)
{
	ls->bad = 1;
	fprintf(stderr, "%s\n", msg);
	longjmp(*ls->recovery, 1);
}

static int issymbol(char c)
{
	return isalnum(c) || c == '_';
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
	} else if (issymbol(c)) {
		/* TODO guard against overflow */
		char buf[100];
		int i = 0;
		do {
			buf[i++] = ls->nextChar;
			ls->nextChar = fgetc(ls->file);
		} while (issymbol(ls->nextChar));
		buf[i] = '\0';
		if (strcmp(buf, "if") == 0) {
			ls->token = TK_KEYWORD_IF;
		} else if (strcmp(buf, "else") == 0) {
			ls->token = TK_KEYWORD_ELSE;
		} else {
			error(ls, "no symbols allowed right now.");
		}
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
	} else if (c == ';') {
		ls->nextChar = fgetc(ls->file);
		ls->token = TK_SEMICOLON;
	} else if (c == '(') {
		ls->nextChar = fgetc(ls->file);
		ls->token = TK_LEFT_PAREN;
	} else if (c == ')') {
		ls->nextChar = fgetc(ls->file);
		ls->token = TK_RIGHT_PAREN;
	} else if (c == '{') {
		ls->nextChar = fgetc(ls->file);
		ls->token = TK_LEFT_BRACE;
	} else if (c == '}') {
		ls->nextChar = fgetc(ls->file);
		ls->token = TK_RIGHT_BRACE;
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

