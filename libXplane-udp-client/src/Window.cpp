#include "Window.h"

#include "Log.h"


static bool s_GLFWInitialized = false;

static void GLFWErrorCallback(int error, const char* description)
{
    LOG_ERROR("GLFW Error ({0}): {1}", error, description);
}

Window* Window::Create(const WindowProperties& properties)
{
    return new Window(properties);
}

Window::Window(const WindowProperties& properties)
{
    m_Data.Title = properties.Title;
    m_Data.Width = properties.Width;
    m_Data.Height = properties.Height;

    if (!s_GLFWInitialized)
    {
        if (!glfwInit())
        {
            LOG_ERROR("Error at glfwInit!");
        }
        s_GLFWInitialized = true;
    }

    m_Window = glfwCreateWindow((int)m_Data.Width, (int)m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr);
    if (!m_Window)
    {
        glfwSetErrorCallback(GLFWErrorCallback);

        LOG_ERROR("Error at glfwCreateWindow!");
        glfwTerminate();
    }

    glfwMakeContextCurrent(m_Window);
    m_Data.Status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSetWindowUserPointer(m_Window, &m_Data);
    setVSync(false);
}

Window::~Window()
{
    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

void Window::onUpdate()
{
    glfwSwapBuffers(m_Window);
    glfwPollEvents();
}

void Window::setVSync(bool enabled)
{
        if (enabled) glfwSwapInterval(1);
        else glfwSwapInterval(0);

        m_Data.VSync = enabled;
}