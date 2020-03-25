// vim: noet ts=4 sw=4
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "error.h"
#include "parse.h"

inline char *_friendly_enum(const LAIR_TOKEN val) {
	switch (val) {
		case LR_ERR:			return "ERR";
		case LR_FUNCTION_CALL:	return "FUNCTION_CALL";
		case LR_FUNCTION_DEF:	return "FUNCTION_DEF";
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
		case LR_IF:				return "IF";
		case LR_BOOL:			return "BOOL";
		case LR_EOF:			return "EOF";
		default:				return "ERR";
	}
}

void lair_print_tokens(const struct _lair_token *tokens) {
	const struct _lair_token *cur_tok = tokens;
	unsigned int last_indent_level = 0;
	while (cur_tok != NULL) {
		const char *frnd = _friendly_enum(cur_tok->token_type);
		switch (cur_tok->token_type) {
			case LR_FUNCTION_DEF:
			case LR_FUNCTION_CALL:
				printf("%s %s ", frnd, cur_tok->token_str);
				break;
			case LR_DEDENT:
				printf("\n}\n");
				break;
			case LR_INDENT: {
				char lvl[cur_tok->indent_level + 1];
				memset(lvl, ' ', cur_tok->indent_level);
				lvl[cur_tok->indent_level] = '\0';
				if (last_indent_level < cur_tok->indent_level) {
					printf("{\n%s", lvl);
				} else {
					printf("\n%s", lvl);
				}
				last_indent_level = cur_tok->indent_level;
				break;
			}
			case LR_ERR:
			default:
				printf("%s ", frnd);
		}
		cur_tok = cur_tok->next;
	}
	printf("\n---\n");
}

static struct _str read_line(const char *buf) {
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

	struct _str to_return = {
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
	unsigned int i = 0;
	for (; i < strlen(token); i++) {
		/* ASCII numerical values are between 0x30 and 0x39. */
		if ((int)token[i] < 0x30 || (int)token[i] > 0x39)
			return 0;
	}
	return 1;
}

static inline void _strip(const char *from, char *to) {
	unsigned int i = 0, j = 0;
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

static void _insert_token(struct _lair_token **head, struct _lair_token *to_insert) {
	if (*head == NULL) {
		*head = to_insert;
		return;
	}

	struct _lair_token *current = *head;
	while (current->next != NULL) {
		current = current->next;
	}
	current->next = to_insert;
	to_insert->prev = current;
}

static int _is_valid_string(const char *stripped, const size_t stripped_len) {
	if (stripped[0] == '"' && stripped[stripped_len - 1] == '"')
		return 1;
	return 0;
}

static int _is_operator(const char *stripped, const size_t stripped_len) {
	if (stripped_len != 0)
		return 0;

	switch (stripped[0]) {
		case '+':
		case '-':
		case '=':
		case '%':
		case '?':
			return 1;
		default:
			return 0;
	}
}

static int _function_args_shadow_function(
		const struct _lair_token *new_token,
		const struct _lair_token **shadowed) {
	const struct _lair_token *cur = new_token->prev;
	while (cur != NULL) {
		if (cur->token_type != LR_FUNCTION_DEF &&
				cur->token_type != LR_FUNCTION_ARG) {
			break;
		}

		const int32_t cmp_result = strcmp(
				new_token->token_str, cur->token_str);
		if (cur->token_str != NULL && cmp_result == 0) {
			*shadowed = cur;
			return 1;
		}

		cur = cur->prev;
	}

	return 0;
}

void _intuit_token_type(
		struct _lair_runtime *r,
		struct _lair_token *new_token,
		const char *stripped) {
	const size_t stripped_len = strlen(stripped);
	if (stripped_len == 1) {
		if (stripped[0] == ':')
			new_token->token_type = LR_RETURN;
		else if (stripped[0] == '!')
			new_token->token_type = LR_CALL;
		else if (stripped[0] == '?')
			new_token->token_type = LR_IF;
		else if (_is_operator(stripped, stripped_len))
			new_token->token_type = LR_OPERATOR;
		else if (_is_valid_string(stripped, stripped_len))
			new_token->token_type = LR_STRING;
		else if (_is_all_numbers(stripped))
			new_token->token_type = LR_NUM;
		else
			new_token->token_type = LR_ATOM;
	} else {
		if (stripped[0] == '"') {
			if (!_is_valid_string(stripped, stripped_len))
				throw_exception(r, ERR_SYNTAX, "String has no ending \".");
			else
				new_token->token_type = LR_STRING;
		} else if (_is_all_numbers(stripped))
			new_token->token_type = LR_NUM;
		else
			new_token->token_type = LR_ATOM;
	}
}

struct _lair_token *_lair_tokenize(struct _lair_runtime *r, const char *program, const size_t len) {
	struct _lair_token *tokens = NULL;
	size_t num_read = 0;

	while (num_read < len) {
		const struct _str line = read_line(program + num_read);
		num_read += line.size;
		int newline = 1;

		/* Read in a token: */
		char *token = strtok((char *)line.data, " ");
		int indentation_level = token - line.data;
		while (token != NULL) {
			/* Is it a comment? Ignore the rest of the line. */
			if (token[0] == '#') {
				break;
			} else if (_is_newline(token)) {
				break;
			}

			/* We don't want to insert an indent/dedent on the very first line,
			* and we only want to do this at the beginning of each line. The beginning
			* is basically the end though so this makes sense, right?
			*/
			if (newline != 0 && tokens != NULL) {
				/* Dedent/indent stuff. */
				struct _lair_token *new_token = calloc(1, sizeof(struct _lair_token));
				new_token->token_str = NULL;
				new_token->indent_level = indentation_level;
				if (token - line.data != 0) {
					/* line starts with spaces. */
					new_token->token_type = LR_INDENT;
				} else {
					new_token->token_type = LR_DEDENT;
				}
				_insert_token(&tokens, new_token);
			}	

			/* Create the shell of the new token and insert it. */
			struct _lair_token *new_token = calloc(1, sizeof(struct _lair_token));
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

#define CALL_OR_FUNCTION if (new_token->token_str[0] == '!' && stripped_len == 1) {\
							new_token->token_type = LR_CALL;\
						} else {\
							new_token->token_type = LR_FUNCTION_DEF;\
						}

			int extra_modified = 0;
			if (new_token->prev != NULL) {
				if (new_token->token_str[0] == '!' && stripped_len == 1) {
					new_token->token_type = LR_CALL;
				} else {
					switch (new_token->prev->token_type) {
						case LR_FUNCTION_DEF:
						case LR_FUNCTION_ARG: {
							const struct _lair_token *out = NULL;
							new_token->token_type = LR_FUNCTION_ARG;
							if (_function_args_shadow_function(new_token, &out)) {
								char buf[512] = {0};
								const char *msg = "Function argument names shadow function name: %s shadows %s";
								snprintf(buf, sizeof(buf), msg, new_token->token_str, (*out).token_str);
								throw_exception(r, ERR_PARSE, buf);
							}
							break;
						}
						case LR_INDENT:
							_intuit_token_type(r, new_token, stripped);
							break;
						case LR_DEDENT:
							CALL_OR_FUNCTION
							break;
						case LR_FUNCTION_CALL:
						default:
							/* Check to see if we hit a space in the middle of a string. */
							if (stripped[0] == '"' && stripped[stripped_len - 1] != '"') {
								// TODO: Rewrite this to support multiline strings. Needs a stateful
								// variable.
								// this is a "test of the thing" okay
								// |--------->    <-----------------|
								const size_t start = token - line.data;
								const size_t end = line.size;
								char remaining[end - start];
								memset(remaining, '\0', end - start);
								memcpy(remaining, stripped, stripped_len);
								remaining[stripped_len] = ' ';
								size_t i = 0;
								size_t new_len = stripped_len + 1;
								int found_end = 0;
								for(i = stripped_len + start + 1; i < end; i++) {
									new_len++;
									remaining[i - start] = line.data[i];

									if (line.data[i] == '"') {
										remaining[i] = '\0';
										found_end = 1;
										break;
									}
								}

								if (!found_end) {
									throw_exception(r, ERR_SYNTAX, "String has no ending \".");
								}

								free(new_token->token_str);
								new_token->token_str = calloc(1, new_len + 1);
								memcpy(new_token->token_str, remaining, new_len);

								_intuit_token_type(r, new_token, remaining);

								extra_modified = 1;
								token = strtok((char *)line.data + start + strlen(remaining), " ");
							} else {
								_intuit_token_type(r, new_token, stripped);
							}
					}
				}
			} else {
				CALL_OR_FUNCTION
			}


			if (extra_modified == 0)
				token = strtok(NULL, " ");
			newline = 0;
		}

		free((char *)line.data);
	}

	struct _lair_token *eof_token = calloc(1, sizeof(struct _lair_token));
	eof_token->token_type = LR_EOF;
	_insert_token(&tokens, eof_token);

	return tokens;
}

static inline struct _lair_token *_pop_token(struct _lair_token **tokens) {
	if (*tokens == NULL)
		return NULL;

	struct _lair_token *top = *tokens;
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
	char *to_return = malloc(new_string_len + 1);
	to_return[new_string_len] = '\0';

	//memcpy(to_return, token + 1, new_string_len);
	int offset = 0;
	size_t i = 0;
	for (i = 0, offset = 1; i < new_string_len; i++, offset++) {
		if (token[offset] != '\\') {
			to_return[i] = token[offset];
		} else {
			switch (token[++offset]) {
				case 'n':
					to_return[i] = '\n';
					break;
				case 'r':
					to_return[i] = '\r';
					break;
				case 't':
					to_return[i] = '\t';
					break;
				default:
					to_return[i] = token[offset];
					break;
			}
		}
	}
	to_return[i] = '\0';
	return to_return;
}

struct _lair_type _lair_atomize_token(const struct _lair_token *token) {
	/* This is where we do parse-time type coersion into things that
	 * better represent the types we're eventually going to want out
	 * of whatever it is they are in.
	 */
	switch (token->token_type) {
		case LR_NUM: {
			struct _lair_type num = {
				.type = LR_NUM,
				.value = {
					.num = atoi(token->token_str)
				}
			};
			return num;
		}
		case LR_STRING: {
			struct _lair_type def = {
				.type = token->token_type,
				.value = {0}
			};
			if (token->token_str != NULL)
				def.value.str = _convert_str_token_to_str(token->token_str);
			return def;
		}
		default: {
			struct _lair_type def = {
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

static void _lair_free_token(struct _lair_token *token) {
	free(token->token_str);
	free(token);
}

void _lair_free_tokens(struct _lair_token *tokens) {
	while (tokens != NULL) {
		struct _lair_token *to_free = (struct _lair_token *)tokens;
		tokens = tokens->next;
		_lair_free_token(to_free);
	}
}

static struct _lair_ast *_parse_from_token(struct _lair_token **tokens) {
	/* "pop" the token off of the top of the stack. */
	struct _lair_token *current_token = _pop_token(tokens);

	/* Atomize the function, stick it at the head of the list. */
	struct _lair_ast _stack_ast = {
		.atom = _lair_atomize_token(current_token),
		.indent_level = current_token->indent_level
	};

	if (current_token->token_type == LR_FUNCTION_CALL ||
			current_token->token_type == LR_FUNCTION_DEF ||
			current_token->token_type == LR_CALL) {
		struct _lair_ast *list = calloc(1, sizeof(struct _lair_ast));
		memcpy(list, &_stack_ast, sizeof(struct _lair_ast));

		/* We break out of the loop when we find an EOF or a DEDENT. */
		struct _lair_ast *cur_ast_item = list;
		struct _lair_ast *prev = NULL;
		while (current_token->token_type != LR_DEDENT &&
			   current_token->token_type != LR_EOF) {
			struct _lair_ast *to_append = _parse_from_token(tokens);
			cur_ast_item->next = to_append;
			cur_ast_item->prev = prev;

			/* Get the next token in line. */
			prev = cur_ast_item;
			cur_ast_item = cur_ast_item->next;
			current_token = (*tokens);
			if (current_token == NULL) {
				/* This happens sometimes if we have a statement like the following:
				 * ! print ! return 10<EOF>
				 */
				return list;
			}
		}

		if (current_token->token_type == LR_EOF) {
			struct _lair_ast *to_append = _parse_from_token(tokens);
			cur_ast_item->next = to_append;
			cur_ast_item->prev = prev;

			prev = cur_ast_item;
			cur_ast_item = cur_ast_item->next;
			current_token = (*tokens);
		}
		return list;
	} else {
		struct _lair_ast *to_return = calloc(1, sizeof(struct _lair_ast));
		memcpy(to_return, &_stack_ast, sizeof(struct _lair_ast));
		_lair_free_token(current_token);
		return to_return;
	}

	_lair_free_tokens(current_token);

	return NULL;
}

struct _lair_ast *_lair_parse_from_tokens(
		struct _lair_runtime *r,
		struct _lair_token **tokens) {
	check(r, tokens != NULL, ERR_PARSE, "No tokens to parse.");
	struct _lair_ast *ast_root = calloc(1, sizeof(struct _lair_ast));
	struct _lair_ast *child_loc = ast_root->children;

	while ((*tokens) != NULL) {
		struct _lair_ast *to_append = _parse_from_token(tokens);

		struct _lair_token *unused = _pop_token(tokens);
		if (unused != NULL)
			_lair_free_token(unused);

		if (ast_root->children == NULL) {
			ast_root->children = to_append;
			child_loc = ast_root->children;
		} else {
			child_loc->sibling = to_append;
			child_loc = child_loc->sibling;
		}
	}

	/* The root node should never have any siblings. */
	check(r, ast_root->next == NULL, ERR_PARSE, "The tree got messed up somehow.");
	return ast_root;
}
