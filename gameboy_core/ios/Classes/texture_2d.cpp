#include "texture_2d.h"
#include <vector>

texture_2d::texture_2d(uint32_t width, uint32_t height)
    : width_(width), height_(height)
{
    this->create_texture();
    this->create_shader();
}

texture_2d::~texture_2d()
{
    glDeleteProgram(this->program_);
    glDeleteTextures(1, &this->texture_);
}

void texture_2d::draw()
{
    GLfloat vertices[] =
    {
        -1.0f,  1.0f, 0.0f, // Position 0
        0.0f,  0.0f,        // TexCoord 0
        -1.0f, -1.0f, 0.0f, // Position 1
        0.0f,  1.0f,        // TexCoord 1
        1.0f, -1.0f, 0.0f,  // Position 2
        1.0f,  1.0f,        // TexCoord 2
        1.0f,  1.0f, 0.0f,  // Position 3
        1.0f,  0.0f         // TexCoord 3
    };
    GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

    glUseProgram(this->program_);
    glVertexAttribPointer(this->position_, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), vertices);
    glVertexAttribPointer(this->tex_coord_, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), &vertices[3]);

    glEnableVertexAttribArray(this->position_);
    glEnableVertexAttribArray(this->tex_coord_);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->texture_);

    glUniform1i(this->sampler_, 0);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
}

void texture_2d::update(const void* buffer)
{
    glBindTexture(GL_TEXTURE_2D, this->texture_);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->width_, this->height_, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
}

void texture_2d::create_texture()
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(1, &this->texture_);
    glBindTexture(GL_TEXTURE_2D, this->texture_);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    std::vector<uint8_t> data(this->width_ * this->height_ * 4);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, this->width_, this->height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());    
}

void texture_2d::create_shader()
{
    const auto v_shader = glCreateShader(GL_VERTEX_SHADER);
    const auto f_shader = glCreateShader(GL_FRAGMENT_SHADER);

    const auto* v_source = R"(
attribute vec4 a_position;
attribute vec2 a_texCoord;
varying vec2 v_texCoord;
void main()
{
    gl_Position = a_position;
    v_texCoord = a_texCoord;
}
)";

    const auto* f_source = R"(
precision mediump float;
varying vec2 v_texCoord;
uniform sampler2D s_texture;
void main()
{
    gl_FragColor = texture2D( s_texture, v_texCoord );
}
)";

    glShaderSource(v_shader, 1, &v_source, 0);
    glShaderSource(f_shader, 1, &f_source, 0);
    glCompileShader(v_shader);
    glCompileShader(f_shader);

    this->program_ = glCreateProgram();
    glAttachShader(this->program_, v_shader);
    glAttachShader(this->program_, f_shader);
    glLinkProgram(this->program_);

    glDeleteShader(v_shader);
    glDeleteShader(f_shader);

    this->position_ = glGetAttribLocation(this->program_, "a_position");
    this->tex_coord_ = glGetAttribLocation(this->program_, "a_texCoord");
    this->sampler_ = glGetUniformLocation(this->program_, "s_texture");
}
