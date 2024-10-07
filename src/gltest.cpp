/*
    Minimal SDL2 + OpenGL3 example.
    Author: https://github.com/koute
    This file is in the public domain; you can do whatever you want with it.
    In case the concept of public domain doesn't exist in your jurisdiction
    you can also use this code under the terms of Creative Commons CC0 license,
    either version 1.0 or (at your option) any later version; for details see:
        http://creativecommons.org/publicdomain/zero/1.0/
    This software is distributed without any warranty whatsoever.
    Compile and run with: gcc opengl3_hello.c `sdl2-config --libs --cflags` -lGL
   -Wall && ./a.out
*/

#define GL_GLEXT_PROTOTYPES

#include <SDL.h>
#include <SDL_opengl.h>

#include <stdio.h>

#include "ibm_font.h"

typedef float t_mat4x4[16];

static inline void mat4x4_ortho(t_mat4x4 out, float left, float right,
                                float bottom, float top, float znear,
                                float zfar) {
#define T(a, b) (a * 4 + b)

  out[T(0, 0)] = 2.0f / (right - left);
  out[T(0, 1)] = 0.0f;
  out[T(0, 2)] = 0.0f;
  out[T(0, 3)] = 0.0f;

  out[T(1, 0)] = 0.0f;
  out[T(1, 1)] = 2.0f / (top - bottom);
  out[T(1, 2)] = 0.0f;
  out[T(1, 3)] = 0.0f;

  out[T(2, 0)] = 0.0f;
  out[T(2, 1)] = 0.0f;
  out[T(2, 2)] = -2.0f / (zfar - znear);
  out[T(2, 3)] = 0.0f;

  out[T(3, 0)] = -(right + left) / (right - left);
  out[T(3, 1)] = -(top + bottom) / (top - bottom);
  out[T(3, 2)] = -(zfar + znear) / (zfar - znear);
  out[T(3, 3)] = 1.0f;

#undef T
}

static const char *vertex_shader =
    "#version 130\n"
    "in vec2 i_position;\n"
    // "in int i_char;\n"
    "in vec4 i_fg_color;\n"
    "in vec4 i_bg_color;\n"
    "out vec4 v_fg_color;\n"
    "out vec4 v_bg_color;\n"
    "uniform mat4 u_projection_matrix;\n"
    "void main() {\n"
    "    v_fg_color = i_fg_color;\n"
    "    v_bg_color = i_bg_color;\n"
    "    gl_Position = u_projection_matrix * vec4(i_position, 0, 1.0);\n"
    "}\n";

static const char *fragment_shader = "#version 130\n"
                                     "in vec4 v_fg_color;\n"
                                     "in vec4 v_bg_color;\n"
                                     "out vec4 o_color;\n"
                                     "void main() {\n"
                                     "    o_color = v_fg_color;\n"
                                     "}\n";

typedef enum t_attrib_id {
  attrib_position,
  attrib_char,
  attrib_fg_color,
  attrib_bg_color
} t_attrib_id;

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  SDL_Init(SDL_INIT_VIDEO);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  static const int width_chars = 80;
  static const int char_cell_width_px = 8;
  static const int width_px = width_chars * char_cell_width_px;
  static const int height_chars = 25;
  static const int char_cell_height_px = 8;
  static const int height_px = height_chars * char_cell_height_px;

  static const int window_width = width_px * 2;
  static const int window_height = height_px * 2 * 12 / 5;

  SDL_Window *window = SDL_CreateWindow(
      "Imbibe", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width,
      window_height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
  SDL_GLContext context = SDL_GL_CreateContext(window);

  GLuint vs, fs, program;

  vs = glCreateShader(GL_VERTEX_SHADER);
  fs = glCreateShader(GL_FRAGMENT_SHADER);

  int length = strlen(vertex_shader);
  glShaderSource(vs, 1, (const GLchar **)&vertex_shader, &length);
  glCompileShader(vs);

  GLint status;
  glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) {
    fprintf(stderr, "vertex shader compilation failed\n");
    return 1;
  }

  length = strlen(fragment_shader);
  glShaderSource(fs, 1, (const GLchar **)&fragment_shader, &length);
  glCompileShader(fs);

  glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) {
    fprintf(stderr, "fragment shader compilation failed\n");
    return 1;
  }

  program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, fs);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
  uint8_t data[256][16][8];
  for (size_t i = 0; i < 256; ++i) {
    for (size_t j = 0; j < 16; ++j) {
      for (size_t k = 0; k < 8; ++k) {
        data[i][j][k] = ibm_font_bits_8x16[i][j] & (1 << (7 - k)) ? 255 : 0;
      }
      // for (size_t k = 8; k < 16; ++k) {
      //   data[i][j][k] = ((i >= 0xC0) && (i <= 0xDF)) ? data[i][j][7] : 0;
      // }
    }
  }
  // glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RED, 8, 16, 256);
  glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RED, 8, 16, 256, GL_RGB,
               GL_UNSIGNED_BYTE, data);
  // glGenerateMipmap(GL_TEXTURE_2D);

  glBindAttribLocation(program, attrib_position, "i_position");
  // glBindAttribLocation(program, attrib_char, "i_char");
  glBindAttribLocation(program, attrib_fg_color, "i_fg_color");
  glBindAttribLocation(program, attrib_bg_color, "i_bg_color");
  glLinkProgram(program);

  glUseProgram(program);

  glDisable(GL_DEPTH_TEST);
  glClearColor(0.5, 0.0, 0.0, 0.0);
  glViewport(0, 0, window_width, window_height);

  GLuint rasterize_vao;
  GLuint rasterize_structure_vbo;
  GLuint rasterize_contents_vbo;

  glGenVertexArrays(1, &rasterize_vao);
  glGenBuffers(1, &rasterize_structure_vbo);
  glGenBuffers(1, &rasterize_contents_vbo);

  glBindVertexArray(rasterize_vao);

  glBindBuffer(GL_ARRAY_BUFFER, rasterize_structure_vbo);

  glEnableVertexAttribArray(attrib_position);
  glEnableVertexAttribArray(attrib_fg_color);
  glEnableVertexAttribArray(attrib_bg_color);

  struct rasterize_structure_t {
    GLfloat fg_color[4];
    GLfloat bg_color[4];
    GLfloat position[2];
  };

  rasterize_structure_t
      g_rasterize_structure_data[width_chars * height_chars * 6];

  for (size_t i = 0; i < height_chars; ++i) {
    for (size_t j = 0; j < width_chars; ++j) {
      for (size_t k = 0; k < 6; ++k) {
        size_t n = (i * width_chars + j) * 6 + k;
        g_rasterize_structure_data[n].fg_color[0] = k > 0;
        g_rasterize_structure_data[n].fg_color[1] = k > 1;
        g_rasterize_structure_data[n].fg_color[2] = k > 2;
        g_rasterize_structure_data[n].fg_color[3] = 1.0f;
        g_rasterize_structure_data[n].bg_color[0] = 0.0f;
        g_rasterize_structure_data[n].bg_color[1] = 0.0f;
        g_rasterize_structure_data[n].bg_color[2] = 0.0f;
        g_rasterize_structure_data[n].bg_color[3] = 1.0f;
      }
      size_t n = (i * width_chars + j) * 6;
      g_rasterize_structure_data[n + 0].position[0] = j * char_cell_width_px;
      g_rasterize_structure_data[n + 0].position[1] = i * char_cell_height_px;
      g_rasterize_structure_data[n + 1].position[0] =
          (j + 1) * char_cell_width_px;
      g_rasterize_structure_data[n + 1].position[1] =
          (i + 0) * char_cell_height_px;
      g_rasterize_structure_data[n + 2].position[0] =
          (j + 1) * char_cell_width_px;
      g_rasterize_structure_data[n + 2].position[1] =
          (i + 1) * char_cell_height_px;
      g_rasterize_structure_data[n + 3].position[0] = j * char_cell_width_px;
      g_rasterize_structure_data[n + 3].position[1] = i * char_cell_height_px;
      g_rasterize_structure_data[n + 4].position[0] =
          (j + 1) * char_cell_width_px;
      g_rasterize_structure_data[n + 4].position[1] =
          (i + 1) * char_cell_height_px;
      g_rasterize_structure_data[n + 5].position[0] =
          (j + 0) * char_cell_width_px;
      g_rasterize_structure_data[n + 5].position[1] =
          (i + 1) * char_cell_height_px;
      // g_rasterize_structure_data[n + 3].position[0] =
      //     (j + 0) * char_cell_width_px;
      // g_rasterize_structure_data[n + 3].position[1] =
      //     (i + 1) * char_cell_height_px;
    }
  }

  glVertexAttribPointer(attrib_fg_color, 4, GL_FLOAT, GL_FALSE,
                        sizeof(rasterize_structure_t),
                        (void *)(offsetof(rasterize_structure_t, fg_color)));
  glVertexAttribPointer(attrib_bg_color, 4, GL_FLOAT, GL_FALSE,
                        sizeof(rasterize_structure_t),
                        (void *)(offsetof(rasterize_structure_t, bg_color)));
  glVertexAttribPointer(attrib_position, 2, GL_FLOAT, GL_FALSE,
                        sizeof(rasterize_structure_t),
                        (void *)(offsetof(rasterize_structure_t, position)));

  glBufferData(GL_ARRAY_BUFFER, sizeof(g_rasterize_structure_data),
               g_rasterize_structure_data, GL_STATIC_DRAW);

  // glBindBuffer(GL_ARRAY_BUFFER, rasterize_contents_vbo);

  // GLubyte g_rasterize_contents_data[width_chars * height_chars * 4][1];
  // for (size_t i = 0; i < height_chars; ++i) {
  //   for (size_t j = 0; j < width_chars; ++j) {
  //     for (size_t k = 0; k < 4; ++k) {
  //       size_t n = (i * width_chars + j) * 4 + k;
  //       g_rasterize_contents_data[n][0] = (GLubyte)n;
  //     }
  //   }
  // }

  // glEnableVertexAttribArray(attrib_char);

  // glVertexAttribPointer(attrib_char, 1, GL_UNSIGNED_BYTE, GL_FALSE,
  //                       sizeof(*g_rasterize_contents_data), (void *)(0));

  // glBufferData(GL_ARRAY_BUFFER, sizeof(g_rasterize_contents_data),
  //              g_rasterize_contents_data, GL_STATIC_DRAW);

  t_mat4x4 projection_matrix;
  mat4x4_ortho(projection_matrix, 0.0f, (float)width_px, (float)height_px, 0.0f,
               0.0f, 1.0f);
  glUniformMatrix4fv(glGetUniformLocation(program, "u_projection_matrix"), 1,
                     GL_FALSE, projection_matrix);

  bool done = false;
  while (!done) {
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(rasterize_vao);
    glDrawArrays(GL_TRIANGLES, 0,  width_chars * height_chars * 6);

    SDL_GL_SwapWindow(window);
    SDL_Delay(1);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT ||
          (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE)) {
        done = true;
      }
    }
  }

  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
