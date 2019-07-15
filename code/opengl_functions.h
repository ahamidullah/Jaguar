#if defined(DEFINEPROC)
#define GLPROC(name, ret, ...)\
	typedef ret (*name##_GLPROC)(__VA_ARGS__);\
	name##_GLPROC name = NULL
#elif defined(LOADPROC)
#define GLPROC(name, ret, ...)\
	name = (name##_GLPROC)SDL_GL_GetProcAddress((const char *)#name);\
	if (!name) _abort("Failed to load OpenGL function %s.", #name)
#else 
#error "Missing DEFINEPROC or LOADPROC."
#endif

GLPROC(glGetUniformBlockIndex, GLuint, GLuint, const GLchar *);
GLPROC(glUniformBlockBinding, void, GLuint, GLuint, GLuint);
GLPROC(glGetUniformLocation, GLint, GLuint, const GLchar *);
GLPROC(glUniformMatrix4fv, void, GLint, GLsizei, GLboolean, const GLfloat *);
GLPROC(glUniform3f, void, GLint, GLfloat, GLfloat, GLfloat);
GLPROC(glUniform4f, void, GLint, GLfloat, GLfloat, GLfloat, GLfloat);
GLPROC(glUniform3fv, void, GLint, GLsizei, const GLfloat *);
GLPROC(glUniform1f, void, GLint, GLfloat);
GLPROC(glUniform1i, void, GLint, GLint);

GLPROC(glGenVertexArrays, void, GLsizei, GLuint *);
GLPROC(glGenBuffers, void, GLsizei, GLuint *);
GLPROC(glBindVertexArray, void, GLuint);
GLPROC(glEnableVertexAttribArray, void, GLuint);
GLPROC(glVertexAttribPointer, void, GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *);
GLPROC(glVertexAttribIPointer, void, GLuint, GLint, GLenum, GLsizei, const GLvoid *);
GLPROC(glBindBuffer, void, GLenum, GLuint);
GLPROC(glBindBufferBase, void, GLenum, GLuint, GLuint);
GLPROC(glBufferSubData, void, GLenum, GLintptr, GLsizeiptr, const GLvoid *);
GLPROC(glBufferData, void, GLenum, GLsizeiptr, const GLvoid *, GLenum);
GLPROC(glDeleteVertexArrays, void, GLsizei, const GLuint *);

GLPROC(glCreateShader, GLuint, GLenum);
GLPROC(glShaderSource, void, GLuint, GLsizei, const GLchar **, const GLint *);
GLPROC(glCompileShader, void, GLuint);
GLPROC(glGetShaderiv, void, GLuint, GLenum, GLint *);
GLPROC(glGetShaderInfoLog, void, GLuint, GLsizei, GLsizei *, GLchar *);
GLPROC(glAttachShader, void, GLuint, GLuint);
GLPROC(glDetachShader, void, GLuint, GLuint);
GLPROC(glDeleteShader, void, GLuint);

GLPROC(glCreateProgram, GLuint, void);
GLPROC(glLinkProgram, void, GLuint);
GLPROC(glGetProgramiv, void, GLuint, GLenum, GLint *);
GLPROC(glGetProgramInfoLog, void, GLuint, GLsizei, GLsizei *, GLchar *);
GLPROC(glUseProgram, void, GLuint);
GLPROC(glGenerateMipmap, void, GLenum);
GLPROC(glDeleteBuffers, void, GLsizei, const GLuint *);

GLPROC(glGenFramebuffers, void, GLsizei, GLuint *);
GLPROC(glBindFramebuffer, void, GLenum, GLuint);
GLPROC(glFramebufferTexture2D, void, GLenum, GLenum, GLenum, GLuint, GLint);

GLPROC(glDebugMessageCallback, void, void (*)(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar*, const void*), void *);
GLPROC(glDebugMessageControl, void, GLenum, GLenum, GLenum, GLsizei, const GLuint *, GLboolean);

#undef GLPROC
#undef DEFINEPROC
#undef LOADPROC
