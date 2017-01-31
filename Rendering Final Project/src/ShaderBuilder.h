#pragma once
#include <glad\glad.h>
#include <iostream>
#include <fstream>

GLuint compile_shaders ( const char* vertexFilename, const char* fragmentFilename );
GLuint compile_shaders( const char* vertexFilename, const char* geometryFilenameconst, const char* fragmentFilename );
GLuint compile_shaders(const char * vertexFilename, const char * tessContFileName, const char * tessEvalFileName, const char * geometryFilename, const char * fragmentFilename);

