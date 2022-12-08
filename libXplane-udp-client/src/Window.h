#pragma once

#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#undef APIENTRY

struct WindowProperties
{
    std::string Title;
    unsigned int Width, Height;

    WindowProperties(const std::string& title, unsigned int width, unsigned int height) :
        Title(title), Width(width), Height(height) { }
};

class Window
{
private:
    GLFWwindow* m_Window;

    Window(const WindowProperties& properties);

    struct WindowData
    {
        std::string Title;
        unsigned int Width, Height;

        int Status;
        bool VSync;
    };

    WindowData m_Data;

public:
    ~Window();

    void onUpdate();

    inline unsigned int getWidth() const { return m_Data.Width; }
    inline unsigned int getHeight() const { return m_Data.Height; }
    inline int getStatus() const { return m_Data.Status; }
    inline GLFWwindow* getGLFWWindow() const { return m_Window; }

    void setVSync(bool enabled);
    inline bool isVSync() const { return m_Data.VSync; };

    static Window* Create(const WindowProperties& properties);
};