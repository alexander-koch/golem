#include "doc.h"

char *trim(char *str)
{
	if(!str) return 0;
	if(str[0] == '\0') return 0;

    size_t len = strlen(str);
    char *front = str;
    char *end = str + len;

	while(isspace(*front))
	{
		++front;
	}

	if(end != front)
	{
		while(isspace(*(--end)) && end != front) {}
	}
	if(str + len - 1 != end)
	{
		*(end + 1) = '\0';
	}
	else if(front != str && end == front)
	{
		*str = '\0';
	}
	end = str;

	if(front != str)
	{
		while(*front)
		{
			*end++ = *front++;
		}
		*end = '\0';
	}
    return str;
}

void doc_val(FILE* fp, val_t val)
{
	if(IS_INT32(val))
	{
		fprintf(fp, "%d", AS_INT32(val));
	}
	else if(IS_NUM(val))
	{
		fprintf(fp, "%f", AS_NUM(val));
	}
	else if(IS_BOOL(val))
	{
		fprintf(fp, "%s", AS_BOOL(val) ? "true" : "false");
	}
	else if(IS_OBJ(val))
	{
		obj_t* obj = AS_OBJ(val);
		if(obj->type == OBJ_STRING)
		{
			fprintf(fp, "%s", (char*)obj->data);
		}
	}
}

void doc_generate(compiler_t* compiler, const char* filename)
{
	size_t len = 0;
	char* source = readFile(filename, &len);

	// Create a new file
	FILE* fp = fopen("doc.html", "wb");

	// HTML header
	fprintf(fp, "<html>\n");
	fprintf(fp, "\t<head>\n");
	fprintf(fp, "\t\t<title>%s Documentation</title>\n", filename);
	fprintf(fp, "\t\t<meta charset=\"utf-8\">\n");
	fprintf(fp, "\t\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n");

	fprintf(fp, "\t\t<style>\n.frame,code{white-space:nowrap}body,code,h1,h2,h3,img,p,pre{margin:0;padding:0}body{background:#fff;color:#333;font:16px/25px \"Source Sans Pro\",georgia,serif}.page{margin:0 auto;width:800px}h2,h3{margin:24px 0 0;color:#9ca0a3}.page:after{content:\"\";display:table;clear:both}.main-column,main{position:relative;width:560px}header h1,header h2{position:absolute;padding:0;letter-spacing:2px}header{border-bottom:solid 1px #141a1f}header .page{height:120px}header h1{left:0;top:63px;font:400 48px Sanchez,helvetica,arial,sans-serif;color:#39c}header h2{right:0;top:72px;font:500 13px Lato,helvetica,arial,sans-serif;text-transform:uppercase;color:#9ca0a3}h1{padding-top:30px;font:500 36px/60px Sanchez,helvetica,arial,sans-serif;color:#9ca0a3}h2{font:500 24px Sanchez,helvetica,arial,sans-serif}h3{font:20px \"Source Sans Pro\",georgia,serif}a{color:#39c;text-decoration:none;transition:color .2s,text-shadow .2s;outline:0}main{padding-top:12px;float:left}li,p{margin:10px 0}p+p{margin-top:20px}code,pre{color:#52667a;font:13px \"Source Code Pro\",Menlo,Monaco,Consolas,monospace;background:#f9fafb;border-radius:1px;border:1px solid #f0f3f5;border-bottom:solid 1px #e0e7eb}code{padding:1px 2px}.frame{text-align:center;margin:1em 0;position:relative}img{margin:0 auto;position:absolute;display:block;height:auto!important;width:auto!important;max-width:100%%}pre{margin:10px 0;line-height:20px;padding:10px;overflow:auto}\n\t\t</style>\n");

	fprintf(fp, "\t</head>\n");
	fprintf(fp, "\t<body>\n");

	// Header title
	fprintf(fp, "\t\t<header>\n");
	fprintf(fp, "\t\t\t<div class=\"page\">\n");
	fprintf(fp, "\t\t\t\t<div class=\"main-column\">\n");
	fprintf(fp, "\t\t\t\t\t<h1>%s</h1>\n", filename);
	fprintf(fp, "\t\t\t\t\t<h2>Documentation</h2>\n");
	fprintf(fp, "\t\t\t\t</div>\n");
	fprintf(fp, "\t\t\t</div>\n");
	fprintf(fp, "\t\t</header>\n\n");
	fprintf(fp, "\t\t<div class=\"page\">\n");
	fprintf(fp, "\t\t<main>\n\n");

	parser_t parser;
	parser_init(&parser, filename);
	ast_t* root = parser_run(&parser, source);
	if(root)
	{
		token_t* buffer = parser.buffer;
		size_t len = parser.num_tokens;
		fprintf(fp, "\t\t<h2>Code</h2>\n\n");
		fprintf(fp, "\t\t<!-- Code segment -->\n");
		fprintf(fp, "\t\t<div>\n<pre>%s</pre>\n\t\t</div>\n", trim(source));

		fprintf(fp, "\t\t<h2>Tokens</h2>\n\n");
		fprintf(fp, "\t\t<!-- Tokens segment -->\n");
		fprintf(fp, "\t\t<div>\n<pre>");
		for(size_t i = 0; i < len; i++)
		{
			token_t* tok = &buffer[i];
			switch(tok->type)
			{
				case TOKEN_NEWLINE:
				{
					fprintf(fp, "NEWLINE,\n");
					continue;
				}
				case TOKEN_SPACE:
				{
					fprintf(fp, "SPACE");
					break;
				}
				default:
				{
					fprintf(fp, "%s", tok->value);
					break;
				}
			}

			if(i < len-1) fprintf(fp, ", ");
		}
		fprintf(fp, "</pre>\n\t\t</div>\n\n");
	}
	ast_free(parser.top);
	parser_free(&parser);
	free(source);

	fprintf(fp, "\t\t<h2>Bytecode</h2>\n\n");
	fprintf(fp, "\t\t<div>\n<pre>");
	vector_t* buffer = compile_file(compiler, filename);
	if(buffer)
	{
		for(size_t i = 0; i < vector_size(buffer); i++)
		{
			instruction_t* instr = vector_get(buffer, i);
			fprintf(fp, "%.3d: %s", (int)i, op2str(instr->op));

			if(instr->v1 != NULL_VAL)
			{
				fprintf(fp, ", ");
				doc_val(fp, instr->v1);
			}
			if(instr->v2 != NULL_VAL)
			{
				fprintf(fp, ", ");
				doc_val(fp, instr->v2);
			}
			fprintf(fp, "\n");
		}
		fprintf(fp, "</pre>\n\t\t</div>\n\n");

		compiler_clear(compiler);
	}

	fprintf(fp, "\t\t</main>\n");
	fprintf(fp, "\t\t</div>\n");
	fprintf(fp, "\t</body>\n");
	fprintf(fp, "</html>\n");

	fclose(fp);
}
