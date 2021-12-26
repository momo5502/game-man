#include <stdint.h>
#include <thread>
#include <string>
#include <atomic>
#include <mutex>
#include <memory>
#include <vector>
#include "gb/game_boy.hpp"

#include <android/log.h>
#include <GLES3/gl31.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

#include "gb/utils/utils.hpp"
#include "egl_thread_surface.h"

#include <dlfcn.h>

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
    std::unique_ptr<egl_thread_surface> thread_surface_{};
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

        char buffer[GB_WIDTH * GB_HEIGHT * 4]{};

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, GB_WIDTH, GB_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);












        // Create shaders
        GLint v_shader = glCreateShader(GL_VERTEX_SHADER);
        GLint f_shader = glCreateShader(GL_FRAGMENT_SHADER);

    const GLchar* vShaderStr =
        "attribute vec4 a_position;   \n"
        "attribute vec2 a_texCoord;   \n"
        "varying vec2 v_texCoord;     \n"
        "void main()                  \n"
        "{                            \n"
        "   gl_Position = a_position; \n"
        "   v_texCoord = a_texCoord;  \n"
        "}                            \n";

    const GLchar* fShaderStr =
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
        if (!this->thread_surface_) {
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

        this->thread_surface_->swap_buffers();
    }

    void set_title(std::string title) override {
        __android_log_write(ANDROID_LOG_INFO, "gbegl", title.data());
    }

    void run(void* data, uint64_t size)
    {
        uint8_t* start = reinterpret_cast<uint8_t*>(data);

        std::vector<uint8_t> buffer{};
        buffer.assign(start, start + size);

        this->runner = std::thread([this, b = std::move(buffer)]() {
            this->work(std::move(b));
        });
    }

private:
    void work(std::vector<uint8_t> buffer) {
        this->thread_surface_ = std::make_unique<egl_thread_surface>(window);

        createTexture();

        gb.load_rom(std::move(buffer));
        gb.skip_bios();

        gb.run();
    }
};

std::mutex m;
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
