// Based on minimal SDL2 + OpenGL3 example by https://github.com/koute

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

static const char *vertex_shader = "#version 330 core\n"
                                   "in vec2 i_cell;\n"
                                   "in uint i_char;\n"
                                   "in vec4 i_fg_color;\n"
                                   "in vec4 i_bg_color;\n"
                                   "out vec2 g_cell;\n"
                                   "out uint g_char;\n"
                                   "out vec4 g_fg_color;\n"
                                   "out vec4 g_bg_color;\n"
                                   "uniform uint u_blink_control;\n"
                                   "void main() {\n"
                                   "    g_cell = i_cell;\n"
                                   "    g_char = i_char;\n"
                                   "    g_fg_color = i_fg_color;\n"
                                   "    g_bg_color = i_bg_color;\n"
                                   "}\n";

static const char *geometry_shader =
    "#version 330 core\n"
    "layout(points) in;\n"
    "layout(triangle_strip, max_vertices = 4) out;\n"
    "in uint g_char[];\n"
    "in vec2 g_cell[];\n"
    "in vec4 g_fg_color[];\n"
    "in vec4 g_bg_color[];\n"
    "out vec4 v_fg_color;\n"
    "out vec4 v_bg_color;\n"
    "uniform vec2 u_cell_size;\n"
    "uniform mat4 u_projection_matrix;\n"
    "void main() {\n"
    "    v_fg_color = g_fg_color[0];\n"
    "    v_bg_color = g_bg_color[0];\n"
    "    gl_Position = u_projection_matrix * vec4(g_cell[0].x * u_cell_size.x, "
    "g_cell[0].y * u_cell_size.y, 0, 1.0);\n"
    "    EmitVertex();\n"
    "    v_fg_color = g_bg_color[0];\n"
    "    v_bg_color = g_fg_color[0];\n"
    "    gl_Position = u_projection_matrix * vec4((g_cell[0].x + 1) * "
    "u_cell_size.x, g_cell[0].y * u_cell_size.y, 0, 1.0);\n"
    "    EmitVertex();\n"
    "    v_fg_color = g_bg_color[0];\n"
    "    v_bg_color = g_fg_color[0];\n"
    "    gl_Position = u_projection_matrix * vec4(g_cell[0].x * u_cell_size.x, "
    "(g_cell[0].y + 1) * u_cell_size.y, 0, 1.0);\n"
    "    EmitVertex();\n"
    "    v_fg_color = g_fg_color[0];\n"
    "    v_bg_color = g_bg_color[0];\n"
    "    gl_Position = u_projection_matrix * vec4((g_cell[0].x + 1) * "
    "u_cell_size.x, (g_cell[0].y + 1) * u_cell_size.y, 0, 1.0);\n"
    "    EmitVertex();\n"
    "    EndPrimitive();\n"
    "}\n";

static const char *fragment_shader = "#version 330 core\n"
                                     "in vec4 v_fg_color;\n"
                                     "in vec4 v_bg_color;\n"
                                     "out vec4 o_color;\n"
                                     "void main() {\n"
                                     "    o_color = v_fg_color;\n"
                                     "}\n";

typedef enum t_attrib_id {
  attrib_cell,
  attrib_char,
  attrib_fg_color,
  attrib_bg_color
} t_attrib_id;

GLuint compile_shader(GLenum shader_type, GLchar const *shader_source) {
  char const *shader_type_name = "<unknown>";

  if (shader_type == GL_VERTEX_SHADER) {
    shader_type_name = "vertex";
  } else if (shader_type == GL_GEOMETRY_SHADER) {
    shader_type_name = "geometry";
  } else if (shader_type == GL_FRAGMENT_SHADER) {
    shader_type_name = "fragment";
  }

  GLuint shader = glCreateShader(shader_type);

  if (shader == 0) {
    return 0;
  }

  GLint length = strlen(geometry_shader);
  glShaderSource(shader, 1, &shader_source, &length);
  glCompileShader(shader);

  GLint status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) {
    GLint log_length = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
    if (log_length > 0) {
      char *log = (char *)malloc(log_length);
      glGetShaderInfoLog(shader, log_length, NULL, log);
      fprintf(stderr, "%s shader compilation failed:\n%s\n", shader_type_name,
              log);
      free(log);
    } else {
      fprintf(stderr, "%s shader compilation failed (not able to get log)\n",
              shader_type_name);
    }
    return 0;
  }

  return shader;
}

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

  GLuint vs = compile_shader(GL_VERTEX_SHADER, vertex_shader);
  GLuint gs = compile_shader(GL_GEOMETRY_SHADER, geometry_shader);
  GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader);

  if (vs == 0 || gs == 0 || fs == 0) {
    return 0;
  }

  GLuint program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, gs);
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

  glBindAttribLocation(program, attrib_cell, "i_cell");
  glBindAttribLocation(program, attrib_char, "i_char");
  glBindAttribLocation(program, attrib_fg_color, "i_fg_color");
  glBindAttribLocation(program, attrib_bg_color, "i_bg_color");
  glLinkProgram(program);

  GLint status;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (status == GL_FALSE) {
    GLint log_length = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
    if (log_length > 0) {
      char *log = (char *)malloc(log_length);
      glGetProgramInfoLog(program, log_length, NULL, log);
      fprintf(stderr, "shader program link failed:\n%s\n", log);
      free(log);
    } else {
      fprintf(stderr, "shader program link failed (not able to get log)\n");
    }
    return 0;
  }

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

  glEnableVertexAttribArray(attrib_cell);
  glEnableVertexAttribArray(attrib_fg_color);
  glEnableVertexAttribArray(attrib_bg_color);

  struct rasterize_structure_t {
    GLfloat fg_color[4];
    GLfloat bg_color[4];
    GLfloat cell[2];
  };

  rasterize_structure_t
      g_rasterize_structure_data[width_chars * height_chars * 6];

  for (size_t i = 0; i < height_chars; ++i) {
    for (size_t j = 0; j < width_chars; ++j) {
      size_t n = i * width_chars + j;
      g_rasterize_structure_data[n].fg_color[0] = 1.0f;
      g_rasterize_structure_data[n].fg_color[1] = 1.0f;
      g_rasterize_structure_data[n].fg_color[2] = 1.0f;
      g_rasterize_structure_data[n].fg_color[3] = 1.0f;
      g_rasterize_structure_data[n].bg_color[0] = 0.0f;
      g_rasterize_structure_data[n].bg_color[1] = 0.0f;
      g_rasterize_structure_data[n].bg_color[2] = 0.0f;
      g_rasterize_structure_data[n].bg_color[3] = 1.0f;
      g_rasterize_structure_data[n].cell[0] = j;
      g_rasterize_structure_data[n].cell[1] = i;
    }
  }

  glVertexAttribPointer(attrib_fg_color, 4, GL_FLOAT, GL_FALSE,
                        sizeof(rasterize_structure_t),
                        (void *)(offsetof(rasterize_structure_t, fg_color)));
  glVertexAttribPointer(attrib_bg_color, 4, GL_FLOAT, GL_FALSE,
                        sizeof(rasterize_structure_t),
                        (void *)(offsetof(rasterize_structure_t, bg_color)));
  glVertexAttribPointer(attrib_cell, 2, GL_FLOAT, GL_FALSE,
                        sizeof(rasterize_structure_t),
                        (void *)(offsetof(rasterize_structure_t, cell)));

  glBufferData(GL_ARRAY_BUFFER, sizeof(g_rasterize_structure_data),
               g_rasterize_structure_data, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, rasterize_contents_vbo);

  GLubyte g_rasterize_contents_data[width_chars * height_chars * 6][1];
  for (size_t i = 0; i < height_chars; ++i) {
    for (size_t j = 0; j < width_chars; ++j) {
      size_t n = i * width_chars + j;
      g_rasterize_contents_data[n][0] = (GLubyte)n;
    }
  }

  glEnableVertexAttribArray(attrib_char);

  glVertexAttribPointer(attrib_char, 1, GL_UNSIGNED_BYTE, GL_FALSE,
                        sizeof(*g_rasterize_contents_data), (void *)(0));

  glBufferData(GL_ARRAY_BUFFER, sizeof(g_rasterize_contents_data),
               g_rasterize_contents_data, GL_STATIC_DRAW);

  glUniform1ui(glGetUniformLocation(program, "u_blink_control"), 1);

  glUniform2f(glGetUniformLocation(program, "u_cell_size"), char_cell_width_px,
              char_cell_height_px);

  t_mat4x4 projection_matrix;
  mat4x4_ortho(projection_matrix, 0.0f, (float)width_px, (float)height_px, 0.0f,
               0.0f, 256.0f);
  glUniformMatrix4fv(glGetUniformLocation(program, "u_projection_matrix"), 1,
                     GL_FALSE, projection_matrix);

  bool done = false;
  while (!done) {
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(rasterize_vao);
    glDrawArrays(GL_POINTS, 0, width_chars * height_chars);

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
