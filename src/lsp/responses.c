#include "lsp.h"
#include "utils/json.h"
#include "utils/lib.h"
#include "version.h"
#include "compiler/compiler_internal.h"

void json_begin()
{
	scratch_buffer_append("{");
}

void json_end()
{
	scratch_buffer_append("}");
}

void json_string(const char* str)
{
	scratch_buffer_printf("\"%s\"", str);
}

void json_bool(bool b)
{
	scratch_buffer_append(b ? "true" : "false");
}

void json_coma()
{
	scratch_buffer_append(",");
}

void json_null()
{
	scratch_buffer_append("null");
}

void json_int(uint32_t i)
{
	scratch_buffer_append_signed_int(i);
}

void json_member(const char* name)
{
	scratch_buffer_printf("\"%s\":", name);
}

void lsp_position(unsigned int row, unsigned int col)
{
	json_begin();
	json_member("line");
	json_int(row);
	json_coma();
	json_member("character");
	json_int(col);
	json_end();
}

void lsp_server_capabilities()
{
	json_begin();

	json_member("positionEncoding");
	json_string("utf-8");
	json_coma();

	json_member("hoverProvider");
	json_bool(true);

	json_end();
}

const char *lsp_initialize(JSONObject *req)
{
	json_resp_begin(req);
	json_begin();

	json_member("capabilities");
	lsp_server_capabilities();
	json_coma();

	json_member("serverInfo");
	json_begin();

	json_member("name");
	json_string("C3 Language Server");
	json_coma();
	json_member("version");
	json_string(COMPILER_VERSION);

	json_end();

	json_end();

	is_initialized = true;
	return json_resp_end(0);
}

const char *lsp_hover(JSONObject *req)
{
	JSONObject *params = json_obj_get(req, "params");
	JSONObject *pos = json_obj_get(params, "position");
	unsigned char col = json_obj_get(pos, "character")->f;
	uint32_t row = json_obj_get(pos, "line")->f;

	char *file_path = (char*) lsp_parse_uri(json_obj_get(json_obj_get(json_obj_get(req, "params"), "textDocument"), "uri")->str);

	lsp_compile(file_path);

	fprintf(debug_output, "----FILE: %s\n", file_path);
	fflush(debug_output);

	CompilationUnit *unit = unit_by_file_path(file_path);
	Decl *decl = find_decl_by_pos_in_unit(unit, row + 1, col + 1);

	json_resp_begin(req);
	json_begin();

	json_member("contents");
	if (decl && decl->decl_kind == DECL_IMPORT)
	{
		scratch_buffer_printf("[{\"language\":\"C3\", \"value\": \"import %s;\"}, \"Import module `%s` into `%s`.\"]", decl->import.module->name->module, decl->import.module->name->module, unit->module->name->module);
		json_coma();
		json_member("range");
		json_begin();
		json_member("start");
		lsp_position(decl->import.path->span.row - 1, decl->import.path->span.col - 1);
		json_coma();
		json_member("end");
		lsp_position(decl->import.path->span.row - 1, decl->import.path->span.col + decl->import.path->span.length - 1);
		json_end();
	}
	else
	{
		json_string("That's the hover text from C3 Language Server !");
	}

	json_end();
	free_arena();
	symtab_destroy();
	memory_release();
	return json_resp_end(0);
}

void lsp_initialized(JSONObject *req)
{
	is_initialized = true;
}

const char *lsp_shutdown(JSONObject *req)
{
	json_resp_begin(req);
	is_shutdown = true;
	json_null();
	return json_resp_end(0);
}

void lsp_exit(JSONObject *req)
{
	fprintf(debug_output, "EXIT %d\n", !is_shutdown);
	fflush(debug_output);

	// 0 if the shutdown request was received before, 1 otherwise
	exit(!is_shutdown);
}

void json_resp_begin(JSONObject *req)
{
	scratch_buffer_clear();
	json_begin();
		json_member("jsonrpc");
		json_string("2.0");
		json_coma();

		json_member("id");
		JSONObject *objId = json_obj_get(req, "id");
		if (objId->type == J_STRING)
		{
			json_string(objId->str);
		}
		else
		{
			json_int((int)objId->f);
		}
		json_coma();
		json_member("result");
}

const char *json_resp_end(uint32_t error)
{
	if (error != 0)
	{
		json_coma();
		json_member("error");
		json_begin();

			json_member("code");
			json_int(error);
			json_coma();

			json_member("message");
			json_string(lsp_error_msg(error));

		json_end();
	}
	json_end();
	return scratch_buffer_to_string();
}