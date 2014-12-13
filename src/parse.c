// vim: noet ts=4 sw=4
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "error.h"
#include "parse.h"

inline char *_friendly_enum(const LAIR_TOKEN val) {
	switch (val) {
		case LR_ERR:			return "ERR";
		case LR_FUNCTION:		return "FUNCTION";
		case LR_FUNCTION_ARG:	return "FUNCTION_ARG";
		case LR_OPERATOR:		return "OP";
		case LR_RETURN:			return "RETURN";
		case LR_VARIABLE:		return "VAR";
		case LR_INDENT:			return "INDENT";
		case LR_DEDENT:			return "DEDENT";
		case LR_STRING:			return "STRING";
		case LR_ATOM:			return "ATOM";
		case LR_NUM:			return "NUM";
		case LR_CALL:			return "CALL";
		default:				return "ERR";
	}
}

void lair_print_tokens(const _lair_token *tokens) {
	const _lair_token *cur_tok = tokens;
	while (cur_tok != NULL) {
		const char *frnd = _friendly_enum(cur_tok->token_type);
		switch (cur_tok->token_type) {
			case LR_FUNCTION:
				printf("%s %s ", frnd, cur_tok->token_str);
				break;
			case LR_DEDENT:
				printf("\n");
				break;
			case LR_INDENT: {
				char lvl[cur_tok->indent_level + 1];
				memset(lvl, ' ', cur_tok->indent_level);
				lvl[cur_tok->indent_level] = '\0';
				printf("\n%s", lvl);
				break;
			}
			case LR_ERR:
			default:
				printf("%s ", frnd);
		}
		cur_tok = cur_tok->next;
	}
}

static _str read_line(const char *buf) {
	char c = '\0';

	size_t num_read = 0;
	while (1) {
		c = buf[num_read];
		num_read++;
		if (c == '\0' || c == '\n' || c == '\r')
			break;
	}

	char *data = calloc(1, num_read + 1);
	strncpy(data, buf, num_read);

	_str to_return = {
		.size = num_read,
		.data = data
	};

	return to_return;
}

static inline int _is_newline(const char *token) {
	if (strncmp(token, "\n", strlen("\n")) == 0 ||
		strncmp(token, "\r", strlen("\r")) == 0)
		return 1;
	return 0;
}

static inline int _is_all_numbers(const char *token) {
	int i = 0;
	for (; i < strlen(token); i++) {
		/* ASCII numerical values are between 0x30 and 0x39. */
		if ((int)token[i] < 0x30 || (int)token[i] > 0x39)
			return 0;
	}
	return 1;
}

static inline void _strip(const char *from, char *to) {
	int i = 0, j = 0;
	for (;i < strlen(from);) {
		while (from[i] == '\n' || from[i] == '\r')
			i++;
		if (i >= strlen(from))
			break;
		to[j] = from[i];
		j++;
		i++;
	}
	to[++j] = '\0';
}

static void _insert_token(_lair_token **head, _lair_token *to_insert) {
	if (*head == NULL) {
		*head = to_insert;
		return;
	}

	_lair_token *current = *head;
	while (current->next != NULL) {
		current = current->next;
	}
	current->next = to_insert;
	to_insert->prev = current;
}

static void _intuit_token_type(_lair_token *new_token, const char *stripped) {
	const size_t stripped_len = strlen(stripped);
	if (stripped_len == 1) {
		if (stripped[0] == ':')
			new_token->token_type = LR_RETURN;
		else if (stripped[0] == '!')
			new_token->token_type = LR_CALL;
		else
			new_token->token_type = LR_OPERATOR;
	} else {
		if (stripped[0] == '"') {
			if (stripped[stripped_len - 1] == '"') {
				new_token->token_type = LR_STRING;
			} else {
				error_and_die(ERR_SYNTAX, "String has no ending \".");
			}
		} else if (_is_all_numbers(stripped)) {
			new_token->token_type = LR_NUM;
		} else {
			new_token->token_type = LR_ATOM;
		}
	}
}

_lair_token *_lair_tokenize(const char *program, const size_t len) {
	_lair_token *tokens = NULL;
	size_t num_read = 0;
	while (num_read < len) {
		const _str line = read_line(program + num_read);
		num_read += line.size;
		int newline = 1;

		/* Read in a token: */
		char *token = strtok((char *)line.data, " ");
		int indentation_level = token - line.data;
		while (token != NULL) {
			/* Is it a comment? Ignore the rest of the line. */
			if (token[0] == '#')
				break;
			else if (_is_newline(token))
				break;

			/* We don't want to insert an indent/dedent on the very first line,
			* and we only want to do this at the beginning of each line. The beginning
			* is basically the end though so this makes sense, right?
			*/
			if (newline != 0 && tokens != NULL) {
				/* Dedent/indent stuff. */
				if (token - line.data != 0) {
					/* line starts with spaces. */
					_lair_token *new_token = calloc(1, sizeof(_lair_token));
					new_token->token_str = NULL;
					new_token->token_type = LR_INDENT;
					new_token->indent_level = indentation_level;
					_insert_token(&tokens, new_token);
				} else {
					_lair_token *new_token = calloc(1, sizeof(_lair_token));
					new_token->token_str = NULL;
					new_token->token_type = LR_DEDENT;
					new_token->indent_level = indentation_level;
					_insert_token(&tokens, new_token);
				}
			}	

			/* Create the shell of the new token and insert it. */
			_lair_token *new_token = calloc(1, sizeof(_lair_token));
			new_token->token_str = calloc(1, strlen(token) + 1);
			new_token->indent_level = indentation_level;

			/* Copy the string representation of the token into the stack. */
			const size_t new_token_len = strlen(token);
			char stripped[new_token_len + 1];
			memset(stripped, '\0', new_token_len + 1);
			_strip(token, stripped);
			const size_t stripped_len = strlen(stripped);
			memcpy(new_token->token_str, stripped, stripped_len);

			/* Actually insert it. */
			_insert_token(&tokens, new_token);

#define CALL_OR_FUNCTION if (new_token->token_str[0] == '!')\
							new_token->token_type = LR_CALL;\
						else\
							new_token->token_type = LR_FUNCTION;\

			if (new_token->prev != NULL) {
				switch (new_token->prev->token_type) {
					case LR_FUNCTION:
					case LR_FUNCTION_ARG:
						new_token->token_type = LR_FUNCTION_ARG;
						break;
					case LR_INDENT:
						_intuit_token_type(new_token, stripped);
						break;
					case LR_DEDENT:
						CALL_OR_FUNCTION
						break;
					default:
						_intuit_token_type(new_token, stripped);
				}
			} else {
				CALL_OR_FUNCTION
			}


			token = strtok(NULL, " ");
			newline = 0;
		}

		free((char *)line.data);
	}

	_lair_token *eof_token = calloc(1, sizeof(_lair_token));
	eof_token->token_type = LR_EOF;
	_insert_token(&tokens, eof_token);

	return tokens;
}

static inline _lair_token *_pop_token(_lair_token **tokens) {
	if (*tokens == NULL)
		return NULL;

	_lair_token *top = *tokens;
	*tokens = (*tokens)->next;

	return top;

}

static char *_convert_str_token_to_str(const char *token) {
	/* This function does stuff like convert escaped characters to their
	 * proper values, remove double quotes, etc.
	 */
	/* TODO: Escape stuff here. Like \r, \n, \t, etc. */
	const size_t string_len = strlen(token);
	const size_t new_string_len = string_len - strlen("\"\"");
	char *to_return = calloc(1, new_string_len + 1);

	memcpy(to_return, token + 1, new_string_len);
	return to_return;
}

static _lair_type _lair_atomize_token(const _lair_token *token) {
	/* This is where we do parse-time type coersion into things that
	 * better represent the types we're eventually going to want out
	 * of whatever it is they are in.
	 */
	switch (token->token_type) {
		case LR_NUM: {
			_lair_type num = {
				.type = LR_NUM,
				.value = {
					.num = atoi(token->token_str)
				}
			};
			return num;
		}
		case LR_STRING: {
			_lair_type def = {
				.type = token->token_type,
				.value = {0}
			};
			if (token->token_str != NULL)
				def.value.str = _convert_str_token_to_str(token->token_str);
			return def;
		}
		default: {
			_lair_type def = {
				.type = token->token_type,
				.value = {0}
			};
			if (token->token_str != NULL) {
				def.value.str = calloc(1, strlen(token->token_str) + 1);
				memcpy(def.value.str, token->token_str, strlen(token->token_str));
			}
			return def;
		}
	}
}

static void _lair_free_token(_lair_token *token) {
	free(token->token_str);
	free(token);
}

void _lair_free_tokens(_lair_token *tokens) {
	while (tokens != NULL) {
		_lair_token *to_free = (_lair_token *)tokens;
		tokens = tokens->next;
		_lair_free_token(to_free);
	}
}

static _lair_ast *_parse_from_token(_lair_token **tokens) {
	/* "pop" the token off of the top of the stack. */
	_lair_token *current_token = _pop_token(tokens);
	if (current_token->token_type == LR_FUNCTION ||
		current_token->token_type == LR_CALL) {
		/* Atomize the function, stick it at the head of the list. */
		_lair_ast *list = calloc(1, sizeof(_lair_ast));
		list->atom = _lair_atomize_token(current_token);

		/* We break out of the loop when we find an EOF or a DEDENT. */
		_lair_ast *cur_ast_item = list;
		while (current_token->token_type != LR_DEDENT &&
			   current_token->token_type != LR_EOF) {
			_lair_ast *to_append = _parse_from_token(tokens);
			cur_ast_item->next = to_append;

			/* Get the next token in line. */
			cur_ast_item = cur_ast_item->next;
			current_token = (*tokens);
			if (current_token == NULL) {
				/* This happens sometimes if we have a statement like the following:
				 * ! print ! return 10<EOF>
				 */
				return list;
			}
		}
		return list;
	} else {
		_lair_ast *to_return = calloc(1, sizeof(_lair_ast));
		to_return->atom = _lair_atomize_token(current_token);
		return to_return;
	}

	_lair_free_tokens(current_token);

	return NULL;
}

_lair_ast *_lair_parse_from_tokens(_lair_token **tokens) {
	check(tokens != NULL, ERR_PARSE, "No tokens to parse.");
	_lair_ast *ast_root = calloc(1, sizeof(_lair_ast));
	_lair_ast *child_loc = ast_root->children;

	while ((*tokens) != NULL) {
		_lair_ast *to_append = _parse_from_token(tokens);

		_lair_token *unused_dedent = _pop_token(tokens);
		_lair_free_token(unused_dedent);

		if (ast_root->children == NULL) {
			ast_root->children = to_append;
			child_loc = ast_root->children;
		} else {
			child_loc->sibling = to_append;
			child_loc = child_loc->sibling;
		}
	}

	/* The root node should never have any siblings. */
	check(ast_root->next == NULL, ERR_PARSE, "The tree got messed up somehow.");
	return ast_root;
}
