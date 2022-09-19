#include "lsp.h"
#include "compiler/compiler_internal.h"

Module *module_by_file_path(const char *path)
{
	VECEACH(global_context.module_list, i)
	{
		Module *mod = global_context.module_list[i];
		VECEACH(mod->units, j)
		{
			if (strcmp(mod->units[j]->file->full_path, path) == 0)
				return mod;
		}
	}
	return NULL;
}

CompilationUnit *unit_by_file_path(const char *path)
{
	VECEACH(global_context.module_list, i)
	{
		Module *mod = global_context.module_list[i];
		VECEACH(mod->units, j)
		{
			if (strcmp(mod->units[j]->file->full_path, path) == 0)
				return mod->units[j];
		}
	}
	return NULL;
}

Decl *find_decl_by_pos(Decl **decls, uint32_t row, unsigned char col)
{
	VECEACH(decls, i)
	{
		fprintf(debug_output, "Search row: %d col: %d check row:%d col: %d\n", row, col, decls[i]->span.row, decls[i]->span.col);
		if (decls[i]->span.row == row && decls[i]->span.col <= col && decls[i]->span.col + decls[i]->span.length >= col )
			return decls[i];
	}
	return NULL;
}

#define FIND_DECL(DECLS) if ((result = find_decl_by_pos(DECLS, row, col)) != NULL) { return result; }
Decl *find_decl_by_pos_in_unit(CompilationUnit *unit, uint32_t row, unsigned char col)
{
	Decl *result = NULL;
	FIND_DECL(unit->imports);
	return NULL;
}