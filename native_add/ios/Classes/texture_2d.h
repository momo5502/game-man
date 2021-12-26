#pragma once

#include <cstdint>
#include <GLES3/gl31.h>

class texture_2d
{
public:
    texture_2d(uint32_t width, uint32_t height);
    ~texture_2d();

    texture_2d(const texture_2d&) = delete;
    texture_2d(texture_2d&&) noexcept = delete;

    texture_2d& operator=(const texture_2d&) = delete;
    texture_2d& operator=(texture_2d&&) noexcept = delete;

    void draw();
    void update(const void* buffer);

private:
    uint32_t width_;
    uint32_t height_;

    GLint position_{};
    GLint tex_coord_{};
    GLint sampler_{};

    GLuint program_{};
    GLuint texture_{};

    void create_texture();
    void create_shader();
};