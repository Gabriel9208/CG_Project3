#include "Error.h"

void GLClearError() {
    while (glGetError() != GL_NO_ERROR);
}

bool GLCheckError(const char* function, const char* file, int line) {
    if (GLenum error = glGetError()) {
        std::cout << "[OpenGL Error] (" << error << ") at " << function << " " << file << ": Line " << line << std::endl;
        return false;
    }
    return true;
}