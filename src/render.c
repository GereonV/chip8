#include "render.h"
#include <stdint.h>
#include <stdlib.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "app.h"

static uint64_t screen __attribute__((vector_size(64 * 32 / 8)));

static float const vertices[] = {
	// bottom right triangle
	-1, -1, // bottom left
	 1, -1, // bottom right
	 1,  1, // top    right
	// top left triangle
	-1, -1, // bottom left
	-1,  1, // top    left
	 1,  1, // top    right
};

static char const vertex_source[] =
	"#version 460 core\n"
	"layout (location = 0) in vec2 inPos;\n"
	"out vec2 uv;\n"
	"void main() {\n"
	"	gl_Position = vec4(inPos, 0, 1);\n"
	"	uv = 0.5 * (inPos + 1);\n"
	"}";

static char const fragment_source[] =
	"#version 460 core\n"
	"in vec2 uv;\n"
	"layout (location = 0) uniform uvec2 screen[32];\n"
	"out vec4 outColor;\n"
	"void main() {\n"
	"	uvec2 row = screen[uint((1 - uv.y) * 32)];\n"
	"	uint index = uint((1 - uv.x) * 64);\n"
	"	uint u = row.y;\n"
	"	if(index > 31) {\n"
	"		u = row.x;\n"
	"		index -= 32;"
	"	}\n"
	"	outColor = vec4((u >> index) & 1);\n"
	"}";

static GLuint create_shader(GLenum type, char const * src) {
	auto shader = glCreateShader(type);
	glShaderSource(shader, 1, &src, nullptr);
	glCompileShader(shader);
	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if(!success) {
		GLint length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		char * buf = malloc((unsigned) length);
		glGetShaderInfoLog(shader, (GLsizei) length, nullptr, buf);
		ERROR(buf, 70);
	}
	return shader;
}

void framebuffer_callback_function(GLFWwindow *, int width, int height) {
	glViewport(0, 0, width, height);
}

void init_render() {
	glfwSetFramebufferSizeCallback(glfwGetCurrentContext(), framebuffer_callback_function);
	if(!gladLoadGL(glfwGetProcAddress))
		ERROR("gladLoadGL() failed", 42);
	GLuint vao; glGenVertexArrays(1, &vao);
	GLuint vbo; glGenBuffers(1, &vbo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, nullptr);
	auto program = glCreateProgram();
	auto vertex   = create_shader(GL_VERTEX_SHADER  , vertex_source);
	auto fragment = create_shader(GL_FRAGMENT_SHADER, fragment_source);
	glAttachShader(program, vertex);
	glAttachShader(program, fragment);
	glLinkProgram(program);
	GLint success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if(!success) {
		GLint length;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
		char * buf = malloc((unsigned) length);
		glGetProgramInfoLog(program, (GLsizei) length, nullptr, buf);
		ERROR(buf, 80);
	}
	glDetachShader(program, vertex);
	glDetachShader(program, fragment);
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	glUseProgram(program);
}

void render() {
	for(int i = 0; i < 32; ++i)
		glUniform2ui(i, screen[i] >> 32, screen[i] & 0xFFFFFFFF);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void clear_screen() {
	screen = (typeof(screen)) {};
}

bool draw_sprite(unsigned char const * sprite_data, unsigned char sprite_height, unsigned char x, unsigned char y) {
	x %= 64;
	y %= 32;
	auto end = y + sprite_height;
	if(end > 32)
		end = 32;
	auto result = false;
	for(; y < end; ++y, ++sprite_data) {
		auto data = (uint64_t) *sprite_data << (64 - 8) >> x;
		if(screen[y] & data)
			result = true;
		screen[y] ^= data;
	}
	return result;
}
