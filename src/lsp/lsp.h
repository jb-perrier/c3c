#pragma once

#include "utils/lib.h"

// forward
typedef struct Module_ Module;
typedef struct JSONObject_ JSONObject;
typedef struct CompilationUnit_ CompilationUnit;
typedef struct Decl_ Decl;
typedef struct BuildOptions_ BuildOptions;

typedef void*(LspAllocator)(size_t);

typedef struct
{
	const char *content_str;
	size_t content_length;
	const char *content_type; // application/vscode-jsonrpc; charset=utf-8
	JSONObject *content_json;
} LSPRequest;

typedef enum
{
	SUCCESS = 0,
	UNSUPPORTED_METHOD = -2,
} LSP_ERROR;

extern bool is_shutdown;
extern bool is_initialized;
extern FILE *debug_output;
extern bool fake_response;

void lsp_start(BuildOptions *options);
void lsp_handle_request(LSPRequest *req);
void lsp_compile(const char *file_path);
void lsp_read_loop(BuildOptions *options);
void lsp_execute(LSPRequest *req);
const char* lsp_error_msg(uint32_t error);
void lsp_create_req(const char *json, LSPRequest *req);
void json_resp_begin(JSONObject *req);
const char *json_resp_end(uint32_t error);

Module *module_by_file_path(const char *path);
CompilationUnit *unit_by_file_path(const char *path);
Decl *find_decl_by_pos(Decl **decls, uint32_t row, unsigned char col);
Decl *find_decl_by_pos_in_unit(CompilationUnit *unit, uint32_t row, unsigned char col);

// responses
const char *lsp_initialize(JSONObject *req);
const char *lsp_hover(JSONObject *req);
const char *lsp_shutdown(JSONObject *req);
void lsp_exit(JSONObject *req);
void lsp_initialized(JSONObject *req);
const char *lsp_parse_uri(const char *uri_str);
