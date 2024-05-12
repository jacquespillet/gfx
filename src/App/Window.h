#pragma once


#include <windows.h>

#include <functional>
#include <vector>

#if GFX_API == GFX_GL
#include <glad/gl.h>
#endif

#if GFX_API == GFX_VK
#define GLFW_INCLUDE_VULKAN
#endif

#define GLFW_EXPOSE_NATIVE_WIN32 1
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "Types.h"

struct GLFWwindow;

namespace app
{
inline void DefaultWindowCallback(const std::string &) {}

enum class mouseButton
{
    _1     =  0,
    _2     =  1,
    _3     =  2,
    _4     =  3,
    _5     =  4,
    _6     =  5,
    _7     =  6,
    _8     =  7,
    LAST   = mouseButton::_8,
    LEFT   = mouseButton::_1,
    RIGHT  = mouseButton::_2,
    MIDDLE = mouseButton::_3,
};

enum class keyCode
{
    UNKNOWN       =  -1,
                        
    SPACE         =  32,
    APOSTROPHE    =  39,
    COMMA         =  44,
    MINUS         =  45,
    PERIOD        =  46,
    SLASH         =  47,
    _0            =  48,
    _1            =  49,
    _2            =  50,
    _3            =  51,
    _4            =  52,
    _5            =  53,
    _6            =  54,
    _7            =  55,
    _8            =  56,
    _9            =  57,
    SEMICOLON     =  59,
    EQUAL         =  61,
    A             =  65,
    B             =  66,
    C             =  67,
    D             =  68,
    E             =  69,
    F             =  70,
    G             =  71,
    H             =  72,
    I             =  73,
    J             =  74,
    K             =  75,
    L             =  76,
    M             =  77,
    N             =  78,
    O             =  79,
    P             =  80,
    Q             =  81,
    R             =  82,
    S             =  83,
    T             =  84,
    U             =  85,
    V             =  86,
    W             =  87,
    X             =  88,
    Y             =  89,
    Z             =  90,
    LEFT_BRACKET  =  91,
    BACKSLASH     =  92,
    RIGHT_BRACKET =  93,
    GRAVE_ACCENT  =  96,
    WORLD_1       = 161,
    WORLD_2       = 162,

    ESCAPE        = 256,
    ENTER         = 257,
    TAB           = 258,
    BACKSPACE     = 259,
    INSERT        = 260,
    DEL           = 261,
    RIGHT         = 262,
    LEFT          = 263,
    DOWN          = 264,
    UP            = 265,
    PAGE_UP       = 266,
    PAGE_DOWN     = 267,
    HOME          = 268,
    END           = 269,
    CAPS_LOCK     = 280,
    SCROLL_LOCK   = 281,
    NUM_LOCK      = 282,
    PRINT_SCREEN  = 283,
    PAUSE         = 284,
    F1            = 290,
    F2            = 291,
    F3            = 292,
    F4            = 293,
    F5            = 294,
    F6            = 295,
    F7            = 296,
    F8            = 297,
    F9            = 298,
    F10           = 299,
    F11           = 300,
    F12           = 301,
    F13           = 302,
    F14           = 303,
    F15           = 304,
    F16           = 305,
    F17           = 306,
    F18           = 307,
    F19           = 308,
    F20           = 309,
    F21           = 310,
    F22           = 311,
    F23           = 312,
    F24           = 313,
    F25           = 314,
    KP_0          = 320,
    KP_1          = 321,
    KP_2          = 322,
    KP_3          = 323,
    KP_4          = 324,
    KP_5          = 325,
    KP_6          = 326,
    KP_7          = 327,
    KP_8          = 328,
    KP_9          = 329,
    KP_DECIMAL    = 330,
    KP_DIVIDE     = 331,
    KP_MULTIPLY   = 332,
    KP_SUBTRACT   = 333,
    KP_ADD        = 334,
    KP_ENTER      = 335,
    KP_EQUAL      = 336,
    LEFT_SHIFT    = 340,
    LEFT_CONTROL  = 341,
    LEFT_ALT      = 342,
    LEFT_SUPER    = 343,
    RIGHT_SHIFT   = 344,
    RIGHT_CONTROL = 345,
    RIGHT_ALT     = 346,
    RIGHT_SUPER   = 347,
    MENU          = 348,
};

struct windowCreateOptions
{
    b8 Resizable=true;
    b8 TitleBar=true;
    b8 MultiSampling=false;
    std::function<void(const std::string&)> ErrorCallback = DefaultWindowCallback;

    v2i Size = v2f(800, 600);
    v2i Position = v2f(-1, -1);
    const char *Title = "MainWindow";

    int VersionMajor=4;
    int VersionMinor=5;
};


struct window
{
public:
    window(const windowCreateOptions &windowCreateOptions);

    std::function<void(window &, v2i)> OnResize;
    std::function<void(window &, keyCode, bool)> OnKeyChanged;
    std::function<void(window &, mouseButton, bool)> OnMouseChanged;
    std::function<void(window &, f64, f64)> OnMousePositionChanged;
    std::function<void(window &, f64, f64)> OnMouseWheelChanged;

    bool ShouldClose() const;
    void PollEvents() const;
    void CheckErrors() const;
    void Present();

    u32 Width, Height;

    std::vector<const char *> GetRequiredExtensions();
 
    GLFWwindow *GetHandle();

    HWND GetNativeWindow();
private:
    GLFWwindow *Handle=nullptr;
};
}