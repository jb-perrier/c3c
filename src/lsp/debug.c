#include "lsp.h"
#include "utils/json.h"

void lsp_create_req(const char *json, LSPRequest *req)
{
	JsonParser jparser;
	json_init_string(&jparser, json, &malloc);
	req->content_str = strdup(json);
	req->content_length = strlen(json);
	req->content_type = strdup("application/vscode-jsonrpc; charset=utf-8");
	req->content_json = json_parse(&jparser);
}