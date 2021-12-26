#include <stdint.h>
#include <thread>
#include <string>
#include <atomic>
#include <mutex>
#include <memory>
#include <vector>
#include "gb/game_boy.hpp"

#include <android/log.h>
#include <EGL/egl.h>
#include <GLES3/gl31.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

#include "gb/utils/utils.hpp"

#include <dlfcn.h>

bool done = false;
EGLNativeWindowType window{};

class joypad_mobile : public joypad {
public:
    virtual bool is_up_pressed() override {
        return states[0];
    }

    virtual bool is_down_pressed() override {
        return states[1];
    }

    virtual bool is_left_pressed() override
    {
        return states[2];
    }

    virtual bool is_right_pressed() override
    {
        return states[3];
    }

    virtual bool is_a_pressed()override
    {
        return states[4];
    }

    virtual bool is_b_pressed()override
    {
        return states[5];
    }

    virtual bool is_start_pressed()override
    {
        return states[6];
    }

    virtual bool is_select_pressed()override
    {
        return states[7];
    }

    void set_state(int id, bool state) {
        states[id] = state;
    }

private:
    std::atomic<bool> states[8]{};
};

class display_mobile : public display {
public:
    void draw_frame(const color* buffer/* GB_WIDTH * GB_HEIGHT */) override {
        memcpy(data, buffer, sizeof(data));
        available.store(true);
    }

    void* get_data() {
        if (available.load()) {
            available.store(false);
        }
        return data;
    }

    void set_title(std::string title) override {
        __android_log_write(ANDROID_LOG_INFO, "gb", title.data());
    }

private:
    static color data[GB_WIDTH * GB_HEIGHT];
    std::atomic<bool> available{ false };
};

color display_mobile::data[GB_WIDTH * GB_HEIGHT]{};

class game_boy_mobile
{
public:
    display_mobile display{};
    joypad_mobile joypad{};
    game_boy gb;
    std::thread runner;

    game_boy_mobile()
        : gb(&joypad, &display)
    {

    }

    ~game_boy_mobile() {
        gb.turn_off();
        if (runner.joinable()) {
            runner.join();
        }
    }

    void run(void* data, uint64_t size)
    {
        std::atomic<bool> running{ false };
        runner = std::thread([&running, this, data, size]() {
            uint8_t* start = reinterpret_cast<uint8_t*>(data);
            std::vector<uint8_t> data;
            data.assign(start, start + size);

            gb.load_rom(std::move(data));
            gb.skip_bios();
            running = true;
            gb.run();
            __android_log_write(ANDROID_LOG_INFO, "term", "Terminating!");
            });

        while (!running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
};

typedef struct
{

    // Attribute locations
    GLint positionLoc;
    GLint texCoordLoc;

    // Sampler location
    GLint samplerLoc;

} UserData;

class game_boy_mobile_egl : public display
{
public:
    bool running{ false };
    EGLDisplay display{ EGL_NO_DISPLAY };
    EGLSurface surface{ EGL_NO_SURFACE };
    joypad_mobile joypad{};
    game_boy gb;
    GLuint program;
    GLuint texture;
    UserData user;
    std::thread runner;

    game_boy_mobile_egl()
        : gb(&joypad, this)
    {

    }

    ~game_boy_mobile_egl() {
        gb.turn_off();
        if (runner.joinable()) {
            runner.join();
        }
    }

    void createTexture() {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

        char buffer[GB_WIDTH * GB_HEIGHT * 4]{};

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, GB_WIDTH, GB_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);












        // Create shaders
        GLint v_shader = glCreateShader(GL_VERTEX_SHADER);
        GLint f_shader = glCreateShader(GL_FRAGMENT_SHADER);

    GLchar* vShaderStr =
        "attribute vec4 a_position;   \n"
        "attribute vec2 a_texCoord;   \n"
        "varying vec2 v_texCoord;     \n"
        "void main()                  \n"
        "{                            \n"
        "   gl_Position = a_position; \n"
        "   v_texCoord = a_texCoord;  \n"
        "}                            \n";

    GLchar* fShaderStr =
        "precision mediump float;                            \n"
        "varying vec2 v_texCoord;                            \n"
        "uniform sampler2D s_texture;                        \n"
        "void main()                                         \n"
        "{                                                   \n"
        "  gl_FragColor = texture2D( s_texture, v_texCoord );\n"
        "}                                                   \n";

        glShaderSource(v_shader, 1, &vShaderStr, 0);
        glShaderSource(f_shader, 1, &fShaderStr, 0);
        glCompileShader(v_shader);
        glCompileShader(f_shader);

        this->program = glCreateProgram();
        glAttachShader(this->program, v_shader);
        glAttachShader(this->program, f_shader);
        glLinkProgram(this->program);

        glDeleteShader(v_shader);
        glDeleteShader(f_shader);

        user.positionLoc = glGetAttribLocation(program, "a_position");
        user.texCoordLoc = glGetAttribLocation(program, "a_texCoord");
        user.samplerLoc = glGetUniformLocation(program, "s_texture");
    }

    void draw_frame(const color* buffer/* GB_WIDTH * GB_HEIGHT */) override {
        if (!running) {
            return;
        }

        //glViewport(0, 0, GB_WIDTH, GB_HEIGHT);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindTexture(GL_TEXTURE_2D, this->texture);

        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GB_WIDTH, GB_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
        






            GLfloat vVertices[] = { -1.0f,  1.0f, 0.0f,  // Position 0
                            0.0f,  0.0f,        // TexCoord 0
                           -1.0f, -1.0f, 0.0f,  // Position 1
                            0.0f,  1.0f,        // TexCoord 1
                            1.0f, -1.0f, 0.0f,  // Position 2
                            1.0f,  1.0f,        // TexCoord 2
                            1.0f,  1.0f, 0.0f,  // Position 3
                            1.0f,  0.0f         // TexCoord 3
                         };
    GLushort indices[] =
    { 0, 1, 2, 0, 2, 3 };

    // Set the viewport
    glViewport(0, 0, GB_WIDTH * 2, GB_HEIGHT * 2);

    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);

    // Use the program object
    glUseProgram(program);

    // Load the vertex position
    glVertexAttribPointer(user.positionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), vVertices);
    // Load the texture coordinate
    glVertexAttribPointer(user.texCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), &vVertices[3]);

    glEnableVertexAttribArray(user.positionLoc);
    glEnableVertexAttribArray(user.texCoordLoc);

    // Bind the texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Set the sampler texture unit to 0
    glUniform1i(user.samplerLoc, 0);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

    eglSwapBuffers(display, surface);
    }

    void set_title(std::string title) override {
        __android_log_write(ANDROID_LOG_INFO, "gbegl", title.data());
    }

    void run(void* data, uint64_t size)
    {
        std::atomic<bool> running{ false };
        runner = std::thread([&running, this, data, size]() {
            uint8_t* start = reinterpret_cast<uint8_t*>(data);
            std::vector<uint8_t> buffer;
            buffer.assign(start, start + size);
            running = true;

            this->work(std::move(buffer));

            __android_log_write(ANDROID_LOG_INFO, "term", "Terminating!");
            });

        while (!running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

private:
    void init_gl()
    {
        running = false;

        __android_log_write(ANDROID_LOG_INFO, "term", "iniiit");

        display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (display == EGL_NO_DISPLAY) {
            __android_log_write(ANDROID_LOG_INFO, "term", "No display!");
            return;
        }

        EGLint version[2];
        if (!eglInitialize(display, &version[0], &version[1])) {
            __android_log_write(ANDROID_LOG_INFO, "term", "No init!");
            return;
        }

        EGLConfig config{};
        EGLint num_configs{};
        EGLint attributes[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,    //Request opengl ES2.0
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_DEPTH_SIZE, 24,
            EGL_NONE
        };

        eglChooseConfig(display, attributes, &config, 1, &num_configs);

        if (!num_configs) {
            __android_log_write(ANDROID_LOG_INFO, "term", utils::va("No config: %X!", eglGetError()));
            return;
        }

        EGLint attributes2[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE,
        };

        auto context = eglCreateContext(display, config, EGL_NO_CONTEXT, attributes2);

        surface = eglCreateWindowSurface(display, config, window, nullptr);

        if (!surface || surface == EGL_NO_SURFACE) {
            __android_log_write(ANDROID_LOG_INFO, "term", "No surface!");
            return;
        }

        if (!eglMakeCurrent(display, surface, surface, context)) {
            __android_log_write(ANDROID_LOG_INFO, "term", "current failed!");
            return;
        }

        createTexture();

        running = true;
        __android_log_write(ANDROID_LOG_INFO, "term", "run!");
    }

    void work(std::vector<uint8_t> buffer) {
        init_gl();

        gb.load_rom(std::move(buffer));
        gb.skip_bios();

        gb.run();
    }
};

std::mutex m;
std::unique_ptr<game_boy_mobile> mobile_gb{};
std::unique_ptr<game_boy_mobile_egl> mobile_gb_egl{};

extern "C" __attribute__((visibility("default"))) __attribute__((used))
void
press_button(int id, bool value)
{
    std::lock_guard<std::mutex> _(m);
    if (mobile_gb_egl) {
        mobile_gb_egl->joypad.set_state(id, value);
    }
}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
void*
get_buffer()
{
    std::lock_guard<std::mutex> _(m);
    //if(!mobile_gb) {
    return nullptr;
    //}

    //return mobile_gb->display.get_data();
}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
void
load_rom(void* data, uint64_t size)
{
    std::lock_guard<std::mutex> _(m);
    mobile_gb_egl = std::make_unique<game_boy_mobile_egl>();
    mobile_gb_egl->run(data, size);
}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
void Java_com_example_native_1add_OpenglTexturePlugin_nativeSetSurface(JNIEnv * jenv, jobject obj, jobject surface) {
    __android_log_write(ANDROID_LOG_INFO, "term", "YEEEEEEEEEEEEEEEEDSSSS");
    window = ANativeWindow_fromSurface(jenv, surface);
    __android_log_write(ANDROID_LOG_INFO, "term", utils::va("Window: %p", window));
}

static struct xxx {
    xxx() {
        __android_log_write(ANDROID_LOG_INFO, "term", "Init");
        auto* sym = dlsym(RTLD_DEFAULT, "Java_com_example_native_1add_OpenglTexturePlugin_nativeSetSurface");
        __android_log_write(ANDROID_LOG_INFO, "term", utils::va("Sym: %p %p", sym, Java_com_example_native_1add_OpenglTexturePlugin_nativeSetSurface));
    }
} _xx;