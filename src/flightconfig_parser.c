#include "actions.h"
#include "conditions.h"
#include "events.h"
#include "flightcfg_parser.h"
#include "numeric.h"
#include "stdlib.h"
#include <ctype.h>
#include <fs/fs.h>
#include <logging/log.h>
#include <stdbool.h>
#include <zephyr.h>

LOG_MODULE_REGISTER(parser, CONFIG_LOG_DEFAULT_LEVEL);

enum token {
	LEFT_PAREN,
	RIGHT_PAREN,
	LEFT_BRACKET,
	RIGHT_BRACKET,
	LEFT_BRACE,
	RIGHT_BRACE,
	COMMA,
	PLUS,
	MINUS,
	DECIMAL,
	ASSIGNMENT_OP,
	IDENTIFIER,
	NUMBER,
	INVALID,
	EF,
};

const char *token_name[] = {
    [LEFT_PAREN] = "LEFT_PAREN",
    [RIGHT_PAREN] = "RIGHT_PAREN",
    [LEFT_BRACKET] = "LEFT_BRACKET",
    [RIGHT_BRACKET] = "RIGHT_BRACKET",
    [LEFT_BRACE] = "LEFT_BRACE",
    [RIGHT_BRACE] = "RIGHT_BRACE",
    [COMMA] = "COMMA",
    [PLUS] = "PLUS",
    [MINUS] = "MINUS",
    [DECIMAL] = "DECIMAL",
    [ASSIGNMENT_OP] = "ASSIGNMENT_OP",
    [IDENTIFIER] = "IDENTIFIER",
    [NUMBER] = "NUMBER",
    [INVALID] = "INVALID",
    [EF] = "EOF",
};

static struct fs_file_t fid;
char msg_buffer[128];
char *read_pos = msg_buffer-1;
int line_count = 1;
static enum token tk;
static const char *tk_start;
static int tk_size;

#define PARSE_ERROR(msg)                                                       \
	LOG_ERR("Error at line %d: %s, token: %d", line_count,                 \
		log_strdup(msg), tk);

char next(void)
{
	read_pos += 1;

	if(read_pos > msg_buffer + ARRAY_SIZE(msg_buffer)) {
		printk("message buffer to small");
		k_oops();
	}
	while (1) {
		if (read_pos > msg_buffer + ARRAY_SIZE(msg_buffer)) {
			return 0;
		}

		int rc = fs_read(&fid, read_pos, 1);
		if (rc == 0) {
			return 0;
		}

		if (*read_pos == '\n') {
			line_count++;
			continue;
		}

		if (isspace((unsigned char)*read_pos)) {
			continue;
		}

		break;
	}
	return *read_pos;
}

static inline void terminate_buffer()
{
	char *term_pos = read_pos + 1;
	if (term_pos >= msg_buffer &&
	    term_pos <= msg_buffer + ARRAY_SIZE(msg_buffer)) {
		*term_pos = 0;
		return;
	}

	LOG_ERR("parse buffer to small");
	k_oops();
}

static inline void clear_buffer() { read_pos = msg_buffer - 1; }

static inline void reset(off_t off)
{
	fs_seek(&fid, off, FS_SEEK_CUR);
	read_pos += off;
	terminate_buffer();
}

static enum token lex(const char **start, size_t *length)
{
	enum token tk_type = INVALID;
	char c = next();

	switch (c) {
	case 0:
		return EF;
	case '[':
		*start = read_pos;
		*length = 1;
		tk_type = LEFT_BRACKET;
		goto terminate;
	case ']':
		*start = read_pos;
		*length = 1;
		tk_type = RIGHT_BRACKET;
		goto terminate;
	case '(':
		*start = read_pos;
		*length = 1;
		tk_type = LEFT_PAREN;
		goto terminate;
	case ')':
		*start = read_pos;
		*length = 1;
		tk_type = RIGHT_PAREN;
		goto terminate;
	case '{':
		*start = read_pos;
		*length = 1;
		tk_type = LEFT_BRACE;
		goto terminate;
	case '}':
		*start = read_pos;
		*length = 1;
		tk_type = RIGHT_BRACE;
		goto terminate;
	case ',':
		*start = read_pos;
		*length = 1;
		tk_type = COMMA;
		goto terminate;
	case '+':
		*start = read_pos;
		*length = 1;
		tk_type = PLUS;
		goto terminate;
	case '-':
		*start = read_pos;
		*length = 1;
		tk_type = MINUS;
		goto terminate;
	case '.':
		*start = read_pos;
		*length = 1;
		tk_type = DECIMAL;
		goto terminate;
	case '=':
		*start = read_pos;
		*length = 1;
		tk_type = ASSIGNMENT_OP;
		goto terminate;
	}

	if (isdigit(c)) {
		*start = read_pos;

		while (1) {
			c = next();
			if (isdigit(c)) {
				continue;
			} else {
				reset(-1);
				tk_type = NUMBER;
				*length = read_pos - *start;
				goto terminate;
			}
		}
	}

	if (isalpha(c) || c == '_') {
		*start = read_pos;
		while (1) {
			c = next();
			if (isalpha(c) || c == '_' || isdigit(c)) {
				continue;
			} else {
				reset(-1);
				tk_type = IDENTIFIER;
				*length = read_pos - *start;
				goto terminate;
			}
		}
	}

	return INVALID;

terminate:
	terminate_buffer();
	return tk_type;
}

void str_lwr(char *s)
{
	for (int i = 0; s[i]; i++) { s[i] = tolower(s[i]); }
}

bool left_paren()
{
	if (tk == LEFT_PAREN) {
		tk = lex(&tk_start, &tk_size);
		return true;
	}

	return false;
}

bool right_paren()
{
	if (tk == RIGHT_PAREN) {
		tk = lex(&tk_start, &tk_size);
		return true;
	}

	return false;
}

bool left_bracket()
{
	if (tk == LEFT_BRACKET) {
		tk = lex(&tk_start, &tk_size);
		return true;
	}

	return false;
}

bool right_bracket()
{
	if (tk == RIGHT_BRACKET) {
		tk = lex(&tk_start, &tk_size);
		return true;
	}

	return false;
}

bool left_brace()
{
	if (tk == LEFT_BRACE) {
		tk = lex(&tk_start, &tk_size);
		return true;
	}

	return false;
}

bool right_brace()
{
	if (tk == RIGHT_BRACE) {
		tk = lex(&tk_start, &tk_size);
		return true;
	}

	return false;
}

bool comma()
{
	if (tk == COMMA) {
		tk = lex(&tk_start, &tk_size);
		return true;
	}

	return false;
}

bool decimal()
{
	if (tk == DECIMAL) {
		tk = lex(&tk_start, &tk_size);
		return true;
	}

	return false;
}

bool plus()
{
	if (tk == PLUS) {
		tk = lex(&tk_start, &tk_size);
		return true;
	}

	return false;
}

bool minus()
{
	if (tk == MINUS) {
		tk = lex(&tk_start, &tk_size);
		return true;
	}

	return false;
}

bool assignment_op()
{
	if (tk == ASSIGNMENT_OP) {
		tk = lex(&tk_start, &tk_size);
		return true;
	}

	return false;
}

bool identifier()
{
	if (tk == IDENTIFIER) {
		tk = lex(&tk_start, &tk_size);
		return true;
	}

	return false;
}

bool number()
{
	if (tk == NUMBER) {
		tk = lex(&tk_start, &tk_size);
		return true;
	}

	return false;
}

bool eof()
{
	if (tk == EF) {
		tk = lex(&tk_start, &tk_size);
		return true;
	}

	return false;
}

bool numeric()
{
	if (!number()) {
		return false;
	}

	if (!decimal()) {
		return true;
	}

	number();
	return true;
}

bool argument_list()
{
	if (!comma()) {
		return true;
	}
	clear_buffer();
	if (!numeric()) {
		if (!identifier()) {
			return false;
			PARSE_ERROR("expected numeric value or identifier");
		}
	}

	while (comma()) {
		clear_buffer();
		if (!numeric()) {
			if (!identifier()) {
				return false;
				PARSE_ERROR(
				    "expected numeric value or identifier");
			}
		}
		clear_buffer();
	}
	return true;
}

bool brace_expression()
{
	if (!left_brace()) {
		return false;
	}

	LOG_INF("brace_expression");

	if (!identifier()) {
		PARSE_ERROR("expected numeric value");
		return false;
	}

	if (!argument_list()) {
		return false;
	}

	if (!right_brace()) {
		PARSE_ERROR("expected right brace");
		return false;
	}
	clear_buffer();
	return true;
}

bool brace_expression_list()
{
	if (!comma()) {
		return true;
	}

	if (!brace_expression()) {
		PARSE_ERROR("expected brace expression");
		return false;
	}

	while (comma()) {
		if (!brace_expression()) {
			PARSE_ERROR("expected brace expression");
			return false;
		}
	}
	return true;
}

bool event_body()
{
	clear_buffer();
	while (identifier()) {
		LOG_INF("assignment identifier: %s", log_strdup(msg_buffer));
		if (!assignment_op()) {
			PARSE_ERROR("missing assignment operator");
			return false;
		}

		clear_buffer();
		if (numeric()) {
			LOG_INF("number: %s", log_strdup(msg_buffer));
			clear_buffer();
			continue;
		} else if (brace_expression()) {
			if (!brace_expression_list()) {
				return false;
			}
			clear_buffer();
			continue;
		}
		PARSE_ERROR("expected numeric or brace-expression list");
	}
	return true;
}

bool event_header(event_type *evt_type)
{
	if (!left_bracket()) {
		return false;
	}

	if (!identifier()) {
		PARSE_ERROR("expected identifier");
		return false;
	}

	LOG_INF("event header identifier %s", log_strdup(msg_buffer));

	if (!right_bracket()) {
		PARSE_ERROR("expected right bracket");
		return false;
	}
	clear_buffer();

	return true;
}

bool prog()
{
	event_type evt;
	if (!event_header(&evt)) {
		return false;
	}

	if (!event_body()) {
		return false;
	}

	while (event_header(&evt)) {
		if (!event_body()) {
			return false;
		}
	}

	if (!eof()) {
		PARSE_ERROR("expected end of file");
		return false;
	}

	return true;
}

int parse(const char *path)
{
	int rc = 0;

	if (fs_open(&fid, path, FS_O_READ) != 0) {
		return -EIO;
	}

	tk = lex(&tk_start, &tk_size);
	rc = prog();

	if (fs_close(&fid) != 0) {
		return -EIO;
	}
	return rc;
}
