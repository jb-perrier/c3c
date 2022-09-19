#include <ctype.h>
#include "lsp.h"
#include "utils/json.h"
#include "utils/lib.h"

#define TYPE_FIELD "Content-Type"
#define LENGTH_FIELD "Content-Length"

int is_header_end(const char *str)
{
	return *str == '\n' && *(str - 1) == '\n';
}

size_t skip_whitespace(const char *str, size_t i)
{
	while (str[i] == ' ')
		i++;
	return i;
}

void parse_header(const char *req_str, LSPRequest *lsp)
{
	size_t size = strlen(req_str);
	size_t i = 0;
	while (1)
	{
		// only 2 fields are present in LSP spec
		if (strncmp(&req_str[i], LENGTH_FIELD, strlen(LENGTH_FIELD)) == 0)
		{
			i += strlen(LENGTH_FIELD) + 1; // skip name + :
			i = skip_whitespace(req_str, i);
			char* end;
			lsp->content_length = strtol(&req_str[i], &end, 10);

			if (end - &req_str[i] < 1)
			{
				// Length is invalid
				//TODO: error
			}
			i += end - &req_str[i];
		}
		else if (strncmp(&req_str[i], TYPE_FIELD, strlen(TYPE_FIELD)) == 0)
		{
			i += strlen(TYPE_FIELD) + 1; // skip name + :
			i = skip_whitespace(req_str, i);
			size_t j = i;
			while (req_str[j] != '\r')
				j++;
			lsp->content_type = str_copy(&req_str[i], j - i);
			i = j;
		}
		else
		{
			// ignore unknown fields
			//TODO: error ?
		}

		if (i + 4 >= size)
		{
			//TODO: error
			return;
		}
		else if (is_header_end(&req_str[i]))
		{
			i += 2;
			break;
		}
		else if (req_str[i] == '\n')
		{
			i += 1;
		}
	}
}

void parse_content(const char *content, LSPRequest *lsp)
{
	JsonParser jparser;
	json_init_string(&jparser, content, &malloc);
	lsp->content_json = json_parse(&jparser);
}

void lsp_read_loop(BuildOptions *options)
{
	LSPRequest lsp;
	char input[4096]; // since the LSP spec only allow 2 header fields, it should be enough
	size_t pos = 0;
	char c;
	while ((c = (char)getchar()))
	{
		input[pos] = c;

		// check for header end \n\n instead of \r\n\r\n
		if (pos >= 1 && is_header_end(&input[pos]))
		{
			parse_header(input, &lsp);

			// we have read ur header, reset the buff
			pos = 0;

			// copy req content
			char *content = malloc(lsp.content_length + 1);
			size_t i = 0;
			while (i < lsp.content_length)
			{
				content[i] = (char)getchar();
				i++;
			}
			content[lsp.content_length] = '\0';

			lsp.content_str = content;
			// content str to json object
			parse_content(content, &lsp);

			//lsp_handle_request(&lsp);
			lsp_execute(&lsp);
			free(content);
		}
		else // no header end has been found
		{
			pos++;
		}
	}
	fclose(debug_output);
}
