#include "glextutil.h"

//WGL
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;

//GL extension function pointers
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;

PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLDELETEBUFFERSPROC glDeleteBuffers;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERSUBDATAPROC glBufferSubData;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLGETBUFFERSUBDATAPROC glGetBufferSubData;

PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;

PFNGLCREATESHADERPROC glCreateShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;

PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLDETACHSHADERPROC glDetachShader;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLDELETEPROGRAMPROC glDeleteProgram;
PFNGLVALIDATEPROGRAMPROC glValidateProgram;

PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM2FPROC glUniform2f;
PFNGLUNIFORM3FPROC glUniform3f;
PFNGLUNIFORM4FPROC glUniform4f;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLUNIFORM2IPROC glUniform2i;
PFNGLUNIFORM3IPROC glUniform3i;
PFNGLUNIFORM4IPROC glUniform4i;
PFNGLUNIFORM1UIPROC glUniform1ui;
PFNGLUNIFORM2UIPROC glUniform2ui;
PFNGLUNIFORM3UIPROC glUniform3ui;
PFNGLUNIFORM4UIPROC glUniform4ui;
PFNGLUNIFORM1FVPROC glUniform1fv;
PFNGLUNIFORM2FVPROC glUniform2fv;
PFNGLUNIFORM3FVPROC glUniform3fv;
PFNGLUNIFORM4FVPROC glUniform4fv;
PFNGLUNIFORM1IVPROC glUniform1iv;
PFNGLUNIFORM2IVPROC glUniform2iv;
PFNGLUNIFORM3IVPROC glUniform3iv;
PFNGLUNIFORM4IVPROC glUniform4iv;
PFNGLUNIFORM1UIVPROC glUniform1uiv;
PFNGLUNIFORM2UIVPROC glUniform2uiv;
PFNGLUNIFORM3UIVPROC glUniform3uiv;
PFNGLUNIFORM4UIVPROC glUniform4uiv;
PFNGLUNIFORMMATRIX2FVPROC glUniformMatrix2fv;
PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
PFNGLUNIFORMMATRIX2X3FVPROC glUniformMatrix2x3fv;
PFNGLUNIFORMMATRIX3X2FVPROC glUniformMatrix3x2fv;
PFNGLUNIFORMMATRIX2X4FVPROC glUniformMatrix2x4fv;
PFNGLUNIFORMMATRIX4X2FVPROC glUniformMatrix4x2fv;
PFNGLUNIFORMMATRIX4X2FVPROC glUniformMatrix3x4fv;
PFNGLUNIFORMMATRIX4X2FVPROC glUniformMatrix4x3fv;

PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;

PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;

PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
PFNGLTEXPARAMETERIIVPROC glTexParameterIiv;
PFNGLTEXPARAMETERIUIVPROC glTexParameterIuiv;
PFNGLGETTEXPARAMETERIIVPROC glGetTexParameterIiv;
PFNGLGETTEXPARAMETERIUIVPROC glGetTexParameterIuiv;
PFNGLACTIVETEXTUREPROC glActiveTexture;
PFNGLTEXIMAGE3DPROC glTexImage3D;

PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;

PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
PFNGLFRAMEBUFFERTEXTUREPROC glFramebufferTexture;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
PFNGLDRAWBUFFERSPROC glDrawBuffers;
PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;

PFNGLBINDFRAGDATALOCATIONPROC glBindFragDataLocation;

PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;

PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex;
PFNGLBINDBUFFERBASEPROC glBindBufferBase;
PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBinding;

HMODULE opengl32Module = NULL;

void* GLEXTLoadFunction(const char* name)
{
    void *p = (void*)wglGetProcAddress(name);
    if(p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1))
    {
        p = (void*)GetProcAddress(opengl32Module, name);
    }
    
    return p;
}

void WGLEXTLoadFunctions()
{
    if(!opengl32Module)
    {
        opengl32Module = LoadLibraryW(L"opengl32.dll");
    }
    
    GLPROCLOAD(PFNWGLCREATECONTEXTATTRIBSARBPROC, wglCreateContextAttribsARB);
    GLPROCLOAD(PFNWGLSWAPINTERVALEXTPROC, wglSwapIntervalEXT);
    
    FreeLibrary(opengl32Module);
    opengl32Module = NULL;
}

void GLEXTLoadFunctions()
{
    if(!opengl32Module)
    {
        opengl32Module = LoadLibraryW(L"opengl32.dll");
    }
    
    GLPROCLOAD(PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays);
    GLPROCLOAD(PFNGLBINDVERTEXARRAYPROC, glBindVertexArray);

    GLPROCLOAD(PFNGLGENBUFFERSPROC, glGenBuffers);
	GLPROCLOAD(PFNGLDELETEBUFFERSPROC, glDeleteBuffers);
    GLPROCLOAD(PFNGLBINDBUFFERPROC, glBindBuffer);
    GLPROCLOAD(PFNGLBUFFERSUBDATAPROC, glBufferSubData);
    GLPROCLOAD(PFNGLBUFFERDATAPROC, glBufferData);
    GLPROCLOAD(PFNGLGETBUFFERSUBDATAPROC, glGetBufferSubData);

    GLPROCLOAD(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer);
    GLPROCLOAD(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray);
    GLPROCLOAD(PFNGLDELETEVERTEXARRAYSPROC, glDeleteVertexArrays);

    GLPROCLOAD(PFNGLCREATESHADERPROC, glCreateShader);
    GLPROCLOAD(PFNGLSHADERSOURCEPROC, glShaderSource);
    GLPROCLOAD(PFNGLCOMPILESHADERPROC, glCompileShader);
    GLPROCLOAD(PFNGLGETSHADERIVPROC, glGetShaderiv);
    GLPROCLOAD(PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog);
    GLPROCLOAD(PFNGLGETPROGRAMIVPROC, glGetProgramiv);
    GLPROCLOAD(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog);

    GLPROCLOAD(PFNGLCREATEPROGRAMPROC, glCreateProgram);
    GLPROCLOAD(PFNGLATTACHSHADERPROC, glAttachShader);
    GLPROCLOAD(PFNGLDETACHSHADERPROC, glDetachShader);
    GLPROCLOAD(PFNGLDELETESHADERPROC, glDeleteShader);
    GLPROCLOAD(PFNGLLINKPROGRAMPROC, glLinkProgram);
    GLPROCLOAD(PFNGLUSEPROGRAMPROC, glUseProgram);
    GLPROCLOAD(PFNGLDELETEPROGRAMPROC, glDeleteProgram);
    GLPROCLOAD(PFNGLVALIDATEPROGRAMPROC, glValidateProgram);

    GLPROCLOAD(PFNGLUNIFORM1FPROC, glUniform1f);
    GLPROCLOAD(PFNGLUNIFORM2FPROC, glUniform2f);
    GLPROCLOAD(PFNGLUNIFORM3FPROC, glUniform3f);
    GLPROCLOAD(PFNGLUNIFORM4FPROC, glUniform4f);
    GLPROCLOAD(PFNGLUNIFORM1IPROC, glUniform1i);
    GLPROCLOAD(PFNGLUNIFORM2IPROC, glUniform2i);
    GLPROCLOAD(PFNGLUNIFORM3IPROC, glUniform3i);
    GLPROCLOAD(PFNGLUNIFORM4IPROC, glUniform4i);
    GLPROCLOAD(PFNGLUNIFORM1UIPROC, glUniform1ui);
    GLPROCLOAD(PFNGLUNIFORM2UIPROC, glUniform2ui);
    GLPROCLOAD(PFNGLUNIFORM3UIPROC, glUniform3ui);
    GLPROCLOAD(PFNGLUNIFORM4UIPROC, glUniform4ui);
    GLPROCLOAD(PFNGLUNIFORM1FVPROC, glUniform1fv);
    GLPROCLOAD(PFNGLUNIFORM2FVPROC, glUniform2fv);
    GLPROCLOAD(PFNGLUNIFORM3FVPROC, glUniform3fv);
    GLPROCLOAD(PFNGLUNIFORM4FVPROC, glUniform4fv);
    GLPROCLOAD(PFNGLUNIFORM1IVPROC, glUniform1iv);
    GLPROCLOAD(PFNGLUNIFORM2IVPROC, glUniform2iv);
    GLPROCLOAD(PFNGLUNIFORM3IVPROC, glUniform3iv);
    GLPROCLOAD(PFNGLUNIFORM4IVPROC, glUniform4iv);
    GLPROCLOAD(PFNGLUNIFORM1UIVPROC, glUniform1uiv);
    GLPROCLOAD(PFNGLUNIFORM2UIVPROC, glUniform2uiv);
    GLPROCLOAD(PFNGLUNIFORM3UIVPROC, glUniform3uiv);
    GLPROCLOAD(PFNGLUNIFORM4UIVPROC, glUniform4uiv);
    GLPROCLOAD(PFNGLUNIFORMMATRIX2FVPROC, glUniformMatrix2fv);
    GLPROCLOAD(PFNGLUNIFORMMATRIX3FVPROC, glUniformMatrix3fv);
    GLPROCLOAD(PFNGLUNIFORMMATRIX4FVPROC, glUniformMatrix4fv);
    GLPROCLOAD(PFNGLUNIFORMMATRIX2X3FVPROC, glUniformMatrix2x3fv);
    GLPROCLOAD(PFNGLUNIFORMMATRIX3X2FVPROC, glUniformMatrix3x2fv);
    GLPROCLOAD(PFNGLUNIFORMMATRIX2X4FVPROC, glUniformMatrix2x4fv);
    GLPROCLOAD(PFNGLUNIFORMMATRIX4X2FVPROC, glUniformMatrix4x2fv);
    GLPROCLOAD(PFNGLUNIFORMMATRIX4X2FVPROC, glUniformMatrix3x4fv);
    GLPROCLOAD(PFNGLUNIFORMMATRIX4X2FVPROC, glUniformMatrix4x3fv);
    
    GLPROCLOAD(PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation);
    
    GLPROCLOAD(PFNGLBINDATTRIBLOCATIONPROC, glBindAttribLocation);
    GLPROCLOAD(PFNGLGETATTRIBLOCATIONPROC, glGetAttribLocation);
    
    GLPROCLOAD(PFNGLGENERATEMIPMAPPROC, glGenerateMipmap);
    GLPROCLOAD(PFNGLTEXPARAMETERIIVPROC, glTexParameterIiv);
    GLPROCLOAD(PFNGLTEXPARAMETERIUIVPROC, glTexParameterIuiv);
    GLPROCLOAD(PFNGLGETTEXPARAMETERIIVPROC, glGetTexParameterIiv);
    GLPROCLOAD(PFNGLGETTEXPARAMETERIUIVPROC, glGetTexParameterIuiv);
    GLPROCLOAD(PFNGLACTIVETEXTUREPROC, glActiveTexture);
    GLPROCLOAD(PFNGLTEXIMAGE3DPROC, glTexImage3D);

    GLPROCLOAD(PFNGLGENFRAMEBUFFERSPROC, glGenFramebuffers);
    GLPROCLOAD(PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer);
    GLPROCLOAD(PFNGLCHECKFRAMEBUFFERSTATUSPROC, glCheckFramebufferStatus);
    GLPROCLOAD(PFNGLDELETEFRAMEBUFFERSPROC, glDeleteFramebuffers);

    GLPROCLOAD(PFNGLGENRENDERBUFFERSPROC, glGenRenderbuffers);
    GLPROCLOAD(PFNGLBINDRENDERBUFFERPROC, glBindRenderbuffer);
    GLPROCLOAD(PFNGLRENDERBUFFERSTORAGEPROC, glRenderbufferStorage);
    GLPROCLOAD(PFNGLFRAMEBUFFERTEXTUREPROC, glFramebufferTexture);
    GLPROCLOAD(PFNGLFRAMEBUFFERRENDERBUFFERPROC, glFramebufferRenderbuffer);
    GLPROCLOAD(PFNGLDRAWBUFFERSPROC, glDrawBuffers);
    GLPROCLOAD(PFNGLDELETERENDERBUFFERSPROC, glDeleteRenderbuffers);

    GLPROCLOAD(PFNGLBINDFRAGDATALOCATIONPROC, glBindFragDataLocation);

    GLPROCLOAD(PFNGLFRAMEBUFFERTEXTURE2DPROC, glFramebufferTexture2D);

    GLPROCLOAD(PFNGLGETUNIFORMBLOCKINDEXPROC, glGetUniformBlockIndex);
    GLPROCLOAD(PFNGLBINDBUFFERBASEPROC, glBindBufferBase);
    GLPROCLOAD(PFNGLUNIFORMBLOCKBINDINGPROC, glUniformBlockBinding);
    
    FreeLibrary(opengl32Module);
    opengl32Module = NULL;
}