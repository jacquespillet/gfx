#include "Window.h"

namespace app
{

window::window(const windowCreateOptions &WindowCreateOptions)
{
    if(glfwInit() != GLFW_TRUE)
    {
        WindowCreateOptions.ErrorCallback("GLFW Context initialization failed");
        return;
    }
#if GRAPHICS_API == VK
    if(glfwVulkanSupported() != GLFW_TRUE)
    {
        WindowCreateOptions.ErrorCallback("Vulkan not supported");
        return;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#else if GRAPHICS_API == GL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, WindowCreateOptions.VersionMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, WindowCreateOptions.VersionMinor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_FALSE);
#endif

    glfwWindowHint(GLFW_DECORATED, WindowCreateOptions.TitleBar);
    glfwWindowHint(GLFW_RESIZABLE, WindowCreateOptions.Resizable);
    
    this->Handle = glfwCreateWindow(WindowCreateOptions.Size.x, WindowCreateOptions.Size.y, WindowCreateOptions.Title, nullptr, nullptr);
    if(this->Handle == nullptr)
    {
        WindowCreateOptions.ErrorCallback("Could not create GLFW window");
        return;
    }

#if GRAPHICS_API == GL
    glfwMakeContextCurrent(this->Handle);
    glfwSwapInterval(1);
#endif

    glfwSetWindowPos(this->Handle, WindowCreateOptions.Position.x, WindowCreateOptions.Position.y);
    glfwSetWindowUserPointer(this->Handle, (void*)this);
    glfwSetWindowSizeCallback(this->Handle, [](GLFWwindow *handle, int width, int height){
        auto &Window = *(window*)glfwGetWindowUserPointer(handle);
        if(Window.OnResize) Window.OnResize(Window, v2i(width, height));
    });
    glfwSetKeyCallback(this->Handle, [](GLFWwindow *handle, int key, int scancode, int action, int mods){
        auto &Window = *(window*)glfwGetWindowUserPointer(handle);
        if(Window.OnKeyChanged) Window.OnKeyChanged(Window, (keyCode)key, action==GLFW_PRESS);
    });
    glfwSetMouseButtonCallback(this->Handle, [](GLFWwindow *handle, int button, int action, int mods){
        auto &Window = *(window*)glfwGetWindowUserPointer(handle);
        if(Window.OnMouseChanged) Window.OnMouseChanged(Window, (mouseButton)button, action==GLFW_PRESS);
    });
}


bool window::ShouldClose() const
{
    return glfwWindowShouldClose(this->Handle);
}

void window::PollEvents() const
{
    glfwPollEvents();
}

void window::CheckErrors() const
{
#if API == GL
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR)
    {
        if(err==GL_INVALID_ENUM) printf("GL_INVALID_ENUM\n");
        if(err==GL_INVALID_VALUE) printf("GL_INVALID_VALUE\n");
        if(err==GL_INVALID_OPERATION) printf("GL_INVALID_OPERATION\n");
        if(err==GL_STACK_OVERFLOW) printf("GL_STACK_OVERFLOW\n");
        if(err==GL_STACK_UNDERFLOW) printf("GL_STACK_UNDERFLOW\n");
        if(err==GL_OUT_OF_MEMORY) printf("GL_OUT_OF_MEMORY\n");
        if(err==GL_INVALID_FRAMEBUFFER_OPERATION) printf("GL_INVALID_FRAMEBUFFER_OPERATION\n");
        if(err==GL_CONTEXT_LOST) printf("GL_CONTEXT_LOST\n");
        if(err==GL_TABLE_TOO_LARGE) printf("GL_TABLE_TOO_LARGE1\n");
    }
#endif
}


void window::Present()
{
#if GRAPHICS_API == GL
    glfwSwapBuffers(this->Handle);
#endif
}

GLFWwindow *window::GetHandle()
{
    return this->Handle;
}

std::vector<const char *> window::GetRequiredExtensions()
{
#if GRAPHICS_API == VK
    uint32_t ExtensionsCount=0;
    const char **Extensions = glfwGetRequiredInstanceExtensions(&ExtensionsCount);
    return std::vector<const char *>(Extensions, Extensions + ExtensionsCount);
#else
    std::vector<const char *> Result;
    return Result;
#endif
}



}