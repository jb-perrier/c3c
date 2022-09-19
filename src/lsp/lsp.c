#include "lsp.h"
#include "utils/json.h"
#include "utils/lib.h"
#include "compiler/compiler.h"
#include "compiler/compiler_internal.h"
#include "build/build_options.h"


#define METHOD(NAME, FUNC) if (strcmp(method, NAME) == 0) { result = FUNC(req->content_json); }
#define NOTIFICATION(NAME, FUNC) if (strcmp(method, NAME) == 0) { FUNC(req->content_json); }

FILE *debug_output = NULL;

bool is_initialized = false;
bool is_shutdown = false;
bool fake_response = false;

void lsp_send(const char *result)
{
	if (!result) return;

	char* resp = scratch_buffer_to_string();
	fprintf(debug_output, "----RESPONSE SENT : %s\n", resp);
	fflush(debug_output);

	if (fake_response) return;

	printf("Content-Length: %d\n\n", scratch_buffer.len);
	printf("%s", resp);
	fflush(stdout);
}

void lsp_execute(LSPRequest *req)
{
	const char *result = NULL;
	const char *method = json_obj_get(req->content_json, "method")->str;
	fprintf(debug_output, "\nREQUEST %s\n", method);
	fprintf(debug_output, "----SIZE %zu\n", req->content_length);
	fprintf(debug_output, "----JSON: %s\n", req->content_str);
	fflush(debug_output);

	METHOD("initialize", lsp_initialize);
	NOTIFICATION("initialized", lsp_initialized);
	METHOD("shutdown", lsp_shutdown);
	NOTIFICATION("exit", lsp_exit);
	METHOD("textDocument/hover", lsp_hover);

	lsp_send(result);
}

void lsp_cleanup()
{
	symtab_destroy();
	memory_release();
}

void lsp_handle_request(LSPRequest *req)
{
	/*int result = setjmp(on_err_jump);
	if (result)
	{
		lsp_cleanup();
	}*/
}

void lsp_start(BuildOptions *options)
{
	debug_output = fopen("C:\\Users\\jimiv\\Desktop\\c3c-lsp-logs.txt", "a");
//	debug_output = stdout;
	fake_response = false;
	fprintf(debug_output, "LSP STARTED\n");
	fflush(debug_output);

//	const char *json = "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"textDocument/hover\",\"params\":{\"textDocument\":{\"uri\":\"file:///c%3A/Users/jimiv/.vscode/extensions/c3-lang/test.c3\"},\"position\":{\"line\":1,\"character\":8}}}";
//	LSPRequest req;
//	lsp_create_req(json, &req);
//	lsp_execute(&req);
//	return;
	lsp_read_loop(options);
}

void lsp_compile(const char *file_path)
{
	char *argv[5];
	argv[0] = "c3c";
	argv[1] = "compile";
	argv[2] = strdup(file_path);
	argv[3] = "-C";
	argv[4] = "--lsp";

	memory_init();

	BuildOptions build_options = parse_arguments(5, (const char**) argv);
	compiler_init(build_options.std_lib_dir);
	compile_target(&build_options);

	CompilationUnit *unit = unit_by_file_path(file_path);
	printf("----MODULE: %s\n", unit->module->name->module);
}