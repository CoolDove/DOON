#include "GLShader.h"
#include <fstream>
#include <stdlib.h>
#include "DoveLog.hpp"
#include <assert.h>

static char* read_file(const char* _path) {
	std::ifstream file(_path);
	if (!file.good()) {
		DLOG_ERROR("failed to read file: \"%s\"", _path);
#ifdef _DEBUG
		assert(true);
#else
		return nullptr;
#endif
	}
	file.seekg(0, std::ios::end);
	size_t size = file.tellg();

	char* buf = new char[size]();
	// memset(buf, 0, size);
	file.seekg(0, std::ios::beg);
	file.read(buf, size);
	file.close();
	
	return buf;
}

static GLint compile_shader(GLuint _shader) {
	glCompileShader(_shader);
	GLint compile_tag_frag = 0;
	glGetShaderiv(_shader, GL_COMPILE_STATUS, &compile_tag_frag);
	if (compile_tag_frag == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(_shader, GL_INFO_LOG_LENGTH, &maxLength);

		char* infoLog = (char*)malloc(maxLength);
		glGetShaderInfoLog(_shader, maxLength, &maxLength, &infoLog[0]);

		glDeleteShader(_shader);

		DLOG_ERROR("%s", infoLog);
		free(infoLog);
	}
	return compile_tag_frag;
}

namespace DGL {
bool Shader::load(const std::string _vert_path, const std::string _frag_path) {
	char* vert_src = read_file(_vert_path.c_str());
	char* frag_src = read_file(_frag_path.c_str());

	GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vert_shader, 1, &vert_src, 0);
	GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag_shader, 1, &frag_src, 0);

	GLint compile_tag_vert = compile_shader(vert_shader);
	GLint compile_tag_frag = compile_shader(frag_shader);
	
	if (!compile_tag_vert || !compile_tag_frag) return false;

	GLuint shader_prog;
	shader_prog = glCreateProgram();
	glAttachShader(shader_prog, vert_shader);
	glAttachShader(shader_prog, frag_shader);

	glLinkProgram(shader_prog);
	glUseProgram(shader_prog);
	glDeleteShader(vert_shader);
	glDeleteShader(frag_shader);

	delete [] vert_src;
	delete [] frag_src;

	id_ = shader_prog;
	return true;
}
}