#include "ShaderBuilder.h"

unsigned long getFileLength(std::ifstream& file)
{
	if (!file.good()) return 0;

	file.seekg(0, std::ios::end);
	unsigned long len = file.tellg();
	file.seekg(std::ios::beg);

	return len;
}

GLchar* loadshader(std::string filename)
{
	std::ifstream file;
	file.open(filename.c_str(), std::ios::in); // opens as ASCII!
	if (!file) return NULL;

	unsigned long len = getFileLength(file);

	if (len == 0) return NULL; // Error: Empty File

	GLchar* ShaderSource = 0;

	ShaderSource = new char[len + 1];

	if (ShaderSource == 0) return NULL; // can't reserve memoryf

										// len isn't always strlen cause some characters are stripped in ascii read...
										// it is important to 0-terminate the real length later, len is just max possible value...
	ShaderSource[len] = 0;

	unsigned int i = 0;
	while (file.good())
	{
		ShaderSource[i] = file.get(); // get character from file.
		if (!file.eof())
			i++;
	}

	ShaderSource[i] = 0; // 0-terminate it at the correct position

	file.close();

	return ShaderSource; // No Error
}

void unloadshader(GLchar** ShaderSource)
{
	if (*ShaderSource != 0)delete[] * ShaderSource;
	*ShaderSource = 0;
}

void attachShader(GLuint &program, const char* fileName, GLuint shaderType)
{
	GLuint shader;
	const GLchar *shaderSource[] = { loadshader(fileName) };

	// Create and compile the vertex shader
	shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, shaderSource, NULL);
	glCompileShader(shader);
	GLint status;

	// Check for compilation errors
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar* strInfoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);

		std::cout << "\n\n" << fileName << std::endl;;	// Added this here to show where the error originated as "vertex shader" doesnt help when there are 4 of them"
		fprintf(stderr, "Compilation error in shader vertex_shader: %s\n", strInfoLog);
		delete[] strInfoLog;
	}

	glAttachShader(program, shader);

	glDeleteShader(shader);
	unloadshader((GLchar**)shaderSource);
}

GLuint compile_shaders(const char* vertexFilename, 
						const char* fragmentFilename)
{
	GLuint program = glCreateProgram();
	attachShader(program, vertexFilename, GL_VERTEX_SHADER);
	attachShader(program, fragmentFilename, GL_FRAGMENT_SHADER);
	glLinkProgram(program);
	return program;
}

GLuint compile_shaders(const char* vertexFilename, 
						const char* geometryFilename, 
						const char* fragmentFilename)
{
	GLuint program = glCreateProgram();
	attachShader(program, vertexFilename, GL_VERTEX_SHADER);
	attachShader(program, geometryFilename, GL_GEOMETRY_SHADER);
	attachShader(program, fragmentFilename, GL_FRAGMENT_SHADER);
	glLinkProgram(program);
	return program;
}

GLuint compile_shaders(const char* vertexFilename, 
						const char* tessContFileName,
						const char* tessEvalFileName,
						const char* geometryFilename, 
						const char* fragmentFilename)
{
	GLuint program = glCreateProgram();
	attachShader(program, vertexFilename, GL_VERTEX_SHADER);
	attachShader(program, tessContFileName, GL_TESS_CONTROL_SHADER);
	attachShader(program, tessEvalFileName, GL_TESS_EVALUATION_SHADER);
	attachShader(program, geometryFilename, GL_GEOMETRY_SHADER);
	attachShader(program, fragmentFilename, GL_FRAGMENT_SHADER);
	glLinkProgram(program);
	return program;
}
