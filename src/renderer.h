#ifndef RENDERER_H
#define RENDERER_H
#include "tools.h"
#include "shader.h"
#include "texture.h"
#include "skybox.h"
#include "objloader.h"
#include "model.h"
#include "camera.h"
#include "fbo.h"
#include "animation.h"

typedef struct RendererSettings
{
  ivec2 render_dim;
  b32 lighting_disabled;
  b32 no_mips;
  b32 motion_blur;
  b32 z_prepass;
  b32 light_cull;
  b32 debug_mode;
  b32 cascaded_render;
  b32 sdf_fonts;
  b32 ssao_on;
  //for the postproc shader
  f32 gamma;
  f32 exposure;
}RendererSettings;

typedef struct RendererPointData
{
    vec3 pos;
    vec4 color;
}RendererPointData;

typedef struct RendererModelData
{
  mat4 model;
  GLuint model_vao;
  u32 model_vertex_count;
  TextureHandle diff;
  TextureHandle spec;
  TextureHandle bump;
  Material *material;
}RendererModelData;

typedef struct RendererFilledRect
{
    vec3 offset;
    vec2 dim; //width and height of rect
    vec4 color;
    i32 texture_unit; //in renderer?
}RendererFilledRect;

typedef struct RendererBillboard
{
    vec3 center;
    vec4 color;
    vec2 dim;
    //tex_coordinates
    //tex_unit
    //maybe add texture info later :)
}RendererBillboard;

typedef struct RendererLine
{
    vec3 start;
    vec3 end;
    vec4 color;
}RendererLine;


typedef struct RendererAnimatedModelData
{
    GLuint vao;
    Texture diff;
    Texture spec;
    
    u32 joint_count;
    Joint *joints;
    u32 vertices_count;
    mat4 model;
}RendererAnimatedModelData;

typedef struct RendererChar
{
    vec3 offset;
    vec2 dim;
    vec2 uv; //where in the bitmap font the character is 
}RendererChar;

typedef struct VisibleIndex
{
    u32 index;
}VisibleIndex;


#define RENDERER_MAX_SHADERS 256
#define RENDERER_MAX_BILLBOARDS 1024 *4
#define RENDERER_MAX_POINT_LIGHTS 1024 
#define RENDERER_BYTES_PER_MODEL sizeof(RendererModelData)
#define RENDERER_MAX_MODELS 256
#define RENDERER_MAX_RECTS 1024 
#define RENDERER_MAX_ANIMATED_MODELS 64
#define RENDERER_MAX_TEXT 512 
#define RENDERER_MAX_POINTS 1024
#define FRUSTUM_CORNERS_COUNT 8
#define RENDERER_CASCADES_COUNT 3

internal f32 billboard_data[] = {
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    -0.5f, 0.5f, 0.0f,
    0.5f, 0.5f, 0.0f,
};

typedef struct Renderer
{

  b32 multisampling;
  u32 multisampling_count;
  b32 depthpeeling;
  u32 depthpeel_count;
  RendererSettings renderer_settings;

  OpenGLFBO *current_fbo;
  OpenGLFBO postproc_fbo;
  OpenGLFBO main_fbo;
  OpenGLFBO ssao_fbo;
  OpenGLFBO ui_fbo;
  OpenGLFBO shadowmap_fbo[3];
  OpenGLFBO depthpeel_fbo;
  

  GLuint postproc_vao;
  GLuint model_vao;
  RendererModelData model_instance_data[RENDERER_MAX_MODELS];
  u32 model_alloc_pos;

  GLuint filled_rect_vao;
  GLuint filled_rect_instance_vbo;
  RendererFilledRect filled_rect_instance_data[RENDERER_MAX_RECTS];
  u32 filled_rect_alloc_pos;

  GLuint billboard_vao;
  GLuint billboard_instance_vbo;
  RendererBillboard billboard_instance_data[RENDERER_MAX_BILLBOARDS];
  u32 billboard_alloc_pos;

  
  GLuint point_vao;
  GLuint point_vbo;
  RendererPointData point_instance_data[RENDERER_MAX_POINTS];
  u32 point_alloc_pos;

  GLuint line_vao;
  GLuint line_instance_vbo;
  RendererLine line_instance_data[RENDERER_MAX_RECTS];
  u32 line_alloc_pos;

  GLuint text_vao;
  GLuint text_instance_vbo;
  RendererChar text_instance_data[RENDERER_MAX_TEXT];
  u32 text_alloc_pos;

  Model test_sphere;


  RendererAnimatedModelData animated_model_instance_data[RENDERER_MAX_ANIMATED_MODELS];
  u32 animated_model_alloc_pos;

  Shader shaders[RENDERER_MAX_SHADERS];
  Camera cam;
  u32 shaders_count;
  PointLight point_lights[RENDERER_MAX_POINT_LIGHTS];
  u32 point_light_count;
  DirLight directional_light;
  Material default_material;
  Skybox skybox;
  Texture white_texture;
  TextureHandle bmf;
  mat4 view;
  mat4 proj;


  //CSM stuff
  mat4 active_lsm; //the lsm that is going to be used for rendering the 'current' FBO (depth map)
  mat4 lsms[RENDERER_CASCADES_COUNT];
  f32 cascade_ends_clip_space[RENDERER_CASCADES_COUNT];

  //forward+ stuff
  GLuint light_buffer;
  GLuint visible_light_indices_buffer;
  GLuint debug_texture;


  //ssao stuff
  vec3 ssao_kernel[64];
}Renderer;

void
renderer_init(Renderer *rend);

void
renderer_begin_frame(Renderer *rend);

void
renderer_end_frame(Renderer *rend);

void renderer_push_line(Renderer *rend, vec3 start, vec3 end, vec4 color);

void renderer_push_model(Renderer *rend, Model *m);

void renderer_push_billboard(Renderer *rend, vec3 center, vec4 color);

void renderer_push_obb_wireframe(Renderer *rend, vec3 center, f32 *u, vec3 e);
#endif
