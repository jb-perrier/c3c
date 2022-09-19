#include "lsp.h"

const char* lsp_error_msg(uint32_t error)
{
	switch (error)
	{
		case UNSUPPORTED_METHOD: return "Unsupported method !";
		default:
			return "Unknown error !";
	}
	UNREACHABLE
}