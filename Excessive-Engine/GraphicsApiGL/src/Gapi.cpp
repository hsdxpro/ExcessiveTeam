#include "Gapi.h"

#include "custom_assert.h"

#include "ShaderProgram.h"
#include "Texture.h"
#include "TextureView.h"
#include "Buffer.h"
#include "UniformBuffer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

#include <iostream>
using namespace std;

#ifdef _MSC_VER
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

extern "C"
EXPORT IGapi* createGraphicsApi()
{
  Gapi* gapi = new Gapi();

  GLenum glew_error = glewInit();
    
  GLenum error;
  while ((error = glGetError()) != GL_NO_ERROR);

  if( glew_error != GLEW_OK )
  {
    cerr << "Error initializing GLEW: " << glewGetErrorString( glew_error ) << endl;
  }

  if( !GLEW_VERSION_4_5 )
  {
    cerr << "Error: GL 4.5 is required" << endl;
  }

  cout << "Vendor: " << glGetString( GL_VENDOR ) << endl;
  cout << "Renderer: " << glGetString( GL_RENDERER ) << endl;
  cout << "OpenGL version: " << glGetString( GL_VERSION ) << endl;
  cout << "GLSL version: " << glGetString( GL_SHADING_LANGUAGE_VERSION ) << endl;

  //use a single global vao
  glCreateVertexArrays( 1, &gapi->global_vao );
  glBindVertexArray( gapi->global_vao );

  return gapi;
}

GLenum func_data[] =
{
  GL_NEVER, GL_LESS, GL_EQUAL, GL_LEQUAL, GL_GREATER, GL_NOTEQUAL, GL_GEQUAL, GL_ALWAYS
};

GLenum stencil_op_data[] = 
{
  GL_KEEP, GL_ZERO, GL_REPLACE, GL_INCR, GL_INCR_WRAP, GL_DECR, GL_DECR_WRAP, GL_INVERT 
};

GLenum blend_eq_data[] =
{
  GL_FUNC_ADD, GL_FUNC_SUBTRACT, GL_FUNC_REVERSE_SUBTRACT, GL_MIN, GL_MAX
};

GLenum blend_func_data[] =
{
  GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR, GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA
};

GLenum raster_mode_data[] = 
{
  GL_POINT, GL_LINE, GL_FILL
};

GLenum raster_face_data[] = 
{
  GL_FRONT, GL_BACK, GL_FRONT_AND_BACK
};

GLenum raster_order_data[] = 
{
  GL_CW, GL_CCW
};

IShaderProgram* Gapi::createShaderProgram()
{
  ShaderProgram* sp = new ShaderProgram();
  sp->id = glCreateProgram();
  return sp;
}

ITexture* Gapi::createTexture(rTextureData* data)
{
  Texture* tex = new Texture();
  tex->target = 0;
  
  ASSERT( data )
  {
    if( data->height > 1 )
    {
      if( data->depth > 1 )
      {
        //3D
        tex->dim = THREE;
        
        if( data->is_layered )
        {
          if( data->is_cubemap )
          {
            tex->target = GL_TEXTURE_CUBE_MAP_ARRAY;
          }
          else
          {
            tex->target = GL_TEXTURE_2D_ARRAY;
          }
        }
        else
        {
          tex->target = GL_TEXTURE_3D;
        }
        
        glCreateTextures( tex->target, 1, &tex->id );
        glTextureStorage3D( tex->id, data->num_levels, texture_internal_formats[data->format], data->width, data->height, data->depth );        
      }
      else
      {
        //2D
        tex->dim = TWO;
        
        if( data->is_layered )
        {
          tex->target = GL_TEXTURE_1D_ARRAY;
        }
        else if( data->is_cubemap )
        {
          tex->target = GL_TEXTURE_CUBE_MAP;
        }
        else
        {
          tex->target = GL_TEXTURE_2D;
        }
        
        glCreateTextures( tex->target, 1, &tex->id );
        glTextureStorage2D( tex->id, data->num_levels, texture_internal_formats[data->format], data->width, data->height );
      }
    }
    else
    {
      //1D
      tex->dim = ONE;
      
      tex->target = GL_TEXTURE_1D;

      glCreateTextures( tex->target, 1, &tex->id );
      glTextureStorage1D( tex->id, data->num_levels, texture_internal_formats[data->format], data->width );
    }
    
    tex->d = *data;
  }
  
  return tex;
}

ITextureView* Gapi::createTextureView(rTextureViewData* data)
{
  TextureView* tex = new TextureView();
  
  ASSERT( data )
  {
    tex->dim = data->dim;
  
    if( data->dim == ONE )
    {
      tex->target = GL_TEXTURE_1D;
    }
    else if( data->dim == TWO )
    {
      if( data->is_layered )
      {
        tex->target = GL_TEXTURE_1D_ARRAY;
      }
      else if( data->is_cubemap )
      {
        tex->target = GL_TEXTURE_CUBE_MAP;
      }
      else
      {
        tex->target = GL_TEXTURE_2D;
      }
    }
    else
    {
      if( data->is_layered )
      {
        if( data->is_cubemap )
        {
          tex->target = GL_TEXTURE_CUBE_MAP_ARRAY;
        }
        else
        {
          tex->target = GL_TEXTURE_2D_ARRAY;
        }
      }
      else
      {
        tex->target = GL_TEXTURE_3D;
      }
    }
  
    //glCreateTextures( tex->target, 1, &tex->id );
    glGenTextures( 1, &tex->id );
    glTextureView(	tex->id,
    tex->target,
    static_cast<Texture*>(data->base_tex)->id,
    texture_internal_formats[data->format],
    data->start_level,
    data->num_levels,
    data->start_layer,
    data->num_layers );
  }
  
  return tex;
}

IVertexBuffer* Gapi::createVertexBuffer(rAllocData* data)
{
  VertexBuffer* vbo = new VertexBuffer();
  glCreateBuffers( 1, &vbo->id );
  
  //glBindBuffer( GL_ARRAY_BUFFER, vbo->id ); //TODO not sure if we need this
  glNamedBufferStorage( vbo->id, data->size, 0, GL_DYNAMIC_STORAGE_BIT | 
                                             (data->is_readable ? GL_MAP_READ_BIT : 0) |
                                             (data->is_writable ? GL_MAP_WRITE_BIT : 0) |
                                             (data->is_persistent ? GL_MAP_PERSISTENT_BIT : 0) |
                                             (data->prefer_cpu_storage ? GL_CLIENT_STORAGE_BIT : 0) );

  /*glBufferStorage( GL_ARRAY_BUFFER, data->size, 0, GL_DYNAMIC_STORAGE_BIT | 
                                             (data->is_readable ? GL_MAP_READ_BIT : 0) |
                                             (data->is_writable ? GL_MAP_WRITE_BIT : 0) |
                                             (data->is_persistent ? GL_MAP_PERSISTENT_BIT : 0) |
                                             (data->prefer_cpu_storage ? GL_CLIENT_STORAGE_BIT : 0) );*/

  //glBufferData( GL_ARRAY_BUFFER, data->size, 0, GL_DYNAMIC_DRAW );

  vbo->adata = *data;
  return vbo;
}

IIndexBuffer* Gapi::createIndexBuffer(rAllocData* data)
{
  IndexBuffer* ibo = new IndexBuffer();
  glCreateBuffers( 1, &ibo->id );
  
  //glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo->id );
  glNamedBufferStorage( ibo->id, data->size, 0, GL_DYNAMIC_STORAGE_BIT | 
                                           (data->is_readable ? GL_MAP_READ_BIT : 0) |
                                           (data->is_writable ? GL_MAP_WRITE_BIT : 0) |
                                           (data->is_persistent ? GL_MAP_PERSISTENT_BIT : 0) |
                                           (data->prefer_cpu_storage ? GL_CLIENT_STORAGE_BIT : 0) );

  /*glBufferStorage( GL_ELEMENT_ARRAY_BUFFER, data->size, 0, GL_DYNAMIC_STORAGE_BIT | 
                                             (data->is_readable ? GL_MAP_READ_BIT : 0) |
                                             (data->is_writable ? GL_MAP_WRITE_BIT : 0) |
                                             (data->is_persistent ? GL_MAP_PERSISTENT_BIT : 0) |
                                             (data->prefer_cpu_storage ? GL_CLIENT_STORAGE_BIT : 0) );*/
  
  //glBufferData( GL_ELEMENT_ARRAY_BUFFER, data->size, 0, GL_DYNAMIC_DRAW );

  ibo->adata = *data;
  return ibo;
}

IUniformBuffer* Gapi::createUniformBuffer(rAllocData* data)
{
  UniformBuffer* ubo = new UniformBuffer();
  glCreateBuffers( 1, &ubo->id );
  
  //glBindBuffer( GL_UNIFORM_BUFFER, ubo->id );
  glNamedBufferStorage( ubo->id, data->size, 0, GL_DYNAMIC_STORAGE_BIT | 
                                           (data->is_readable ? GL_MAP_READ_BIT : 0) |
                                           (data->is_writable ? GL_MAP_WRITE_BIT : 0) |
                                           (data->is_persistent ? GL_MAP_PERSISTENT_BIT : 0) |
                                           (data->prefer_cpu_storage ? GL_CLIENT_STORAGE_BIT : 0) );
  
  /*glBufferStorage( GL_UNIFORM_BUFFER, data->size, 0, GL_DYNAMIC_STORAGE_BIT | 
                                             (data->is_readable ? GL_MAP_READ_BIT : 0) |
                                             (data->is_writable ? GL_MAP_WRITE_BIT : 0) |
                                             (data->is_persistent ? GL_MAP_PERSISTENT_BIT : 0) |
                                             (data->prefer_cpu_storage ? GL_CLIENT_STORAGE_BIT : 0) );*/

  //glBufferData( GL_UNIFORM_BUFFER, data->size, 0, GL_DYNAMIC_DRAW );

  ubo->adata = *data;
  return ubo;
}

void Gapi::setDepthState(rDepthState* state)
{
  ASSERT( state )
  {
    if( state->depth_test )
    {
      glEnable( GL_DEPTH_TEST );
    }
    else
    {
      glDisable( GL_DEPTH_TEST );
    }
    
    glDepthMask( state->depth_mask );
    
    glDepthRangef( state->near, state->far );
    
    glDepthFunc( func_data[state->func] );
  }
}

void Gapi::setStencilState(rStencilState* state)
{
  ASSERT( state )
  {
    if( state->stencil_test )
    {
      glEnable( GL_STENCIL_TEST );
    }
    else
    {
      glDisable( GL_STENCIL_TEST );
    }
    
    glDepthMask( state->stencil_mask );
    
    glStencilMask( state->mask );
    
    glStencilFunc( func_data[state->func], state->reference_stencil_value, state->func_mask );
    
    glStencilOp( stencil_op_data[state->on_stencil_fail], stencil_op_data[state->on_stencil_pass_depth_fail], stencil_op_data[state->on_stencil_pass_depth_pass] );
  }
}

void Gapi::setBlendState(rBlendState* state)
{ 
  ASSERT( state )
  {
    if( state->blend_test )
    {
      glEnable( GL_BLEND );
    }
    else
    {
      glDisable( GL_BLEND );
    }
    
    glBlendColor( state->blend_color.x, state->blend_color.y, state->blend_color.z, state->blend_color.w );
    
    glBlendEquation( blend_eq_data[state->equation] );
    
    glBlendFunc( blend_func_data[state->src_func], blend_func_data[state->dst_func] );
  }
}

void Gapi::setSRGBWrites(bool val)
{
  if( val )
  {
    glEnable( GL_FRAMEBUFFER_SRGB );
  }
  else
  {
    glDisable( GL_FRAMEBUFFER_SRGB );
  }
}

void Gapi::setSeamlessCubeMaps(bool val)
{
  if( val )
  {
    glEnable( GL_TEXTURE_CUBE_MAP_SEAMLESS );
  }
  else
  {
    glDisable( GL_TEXTURE_CUBE_MAP_SEAMLESS );
  }
}

void Gapi::setViewport(int x, int y, unsigned w, unsigned h)
{
  glViewport( x, y, w, h );
}

void Gapi::setRasterizationState(rRasterizerState* state)
{
  glPolygonMode(GL_FRONT_AND_BACK, raster_mode_data[state->mode]);
  glFrontFace( raster_order_data[state->vertex_order] );
  glCullFace( raster_face_data[state->face] );
  glColorMask( state->r_mask, state->g_mask, state->b_mask, state->a_mask );
}

bool Gapi::getError() //true if error
{
  return glGetError != GL_NO_ERROR;
}

void Gapi::setDebugOutput(bool val)
{
  if( val )
  {
    glEnable(GL_DEBUG_OUTPUT);
  }
  else
  {
    glDisable(GL_DEBUG_OUTPUT);
  }
}

void Gapi::setSyncDebugOutput(bool val)
{
  if( val )
  {
    glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
  }
  else
  {
    glDisable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
  }
}

void Gapi::setShaderProgram(IShaderProgram* sp)
{
  ASSERT(sp);
  glUseProgram(static_cast<ShaderProgram*>(sp)->id);
}

void Gapi::passTextureView(ITextureView* tex, unsigned index)
{
  ASSERT( tex );
  glBindTextureUnit( index, static_cast<TextureView*>(tex)->id );
}

void Gapi::passRenderTargets(rTargetData* render_targets, unsigned size)
{
  //TODO
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void Gapi::passUniformBuffer(IUniformBuffer* buf, unsigned index)
{
  ASSERT( buf );
  glBindBufferBase( GL_UNIFORM_BUFFER, index, static_cast<UniformBuffer*>(buf)->id );
}

GLenum attrib_array[] = 
{
  GL_FLOAT, GL_INT, GL_UNSIGNED_INT
};

void Gapi::passVertexBuffers(IVertexBuffer** vbos, rVertexAttrib* attrib_data, unsigned num_vbos)
{
  ASSERT( vbos && attrib_data );

  for( int c = 0; c < num_vbos; ++c )
  {
    GLuint id = static_cast<VertexBuffer*>(vbos[c])->id;
    /*glBindBuffer( GL_ARRAY_BUFFER, id );
    glEnableVertexAttribArray( attrib_data[c].index );
    glVertexAttribPointer( attrib_data[c].index, attrib_data[c].data_components, attrib_array[attrib_data[c].type], false, attrib_data[c].size, (const void*)attrib_data[c].offset );*/
    //glVertexAttribDivisor( attrib_data[c].index, attrib_data[c].divisor ); //instancing stuff

    glBindBuffer( GL_ARRAY_BUFFER, id );
    glEnableVertexArrayAttrib( global_vao, attrib_data[c].index );
    glVertexAttribPointer( attrib_data[c].index, attrib_data[c].data_components, attrib_array[attrib_data[c].type], false, attrib_data[c].size, (const void*)attrib_data[c].offset );
    //glVertexArrayVertexBuffer( global_vao, attrib_data[c].index, id, attrib_data[c].offset, 0 );
    //glVertexArrayAttribFormat( global_vao, attrib_data[c].index, attrib_data[c].data_components, attrib_array[attrib_data[c].type], false, attrib_data[c].offset );
    //glVertexArrayAttribBinding( global_vao, attrib_data[c].index, attrib_data[c].index );
    //glVertexArrayBindingDivisor( global_vao, attrib_data[c].index, attrib_data[c].divisor ); //instancing stuff
  }
}

void Gapi::passIndexBuffer(IIndexBuffer* ibo)
{
  ASSERT( ibo );
  //glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, static_cast<IndexBuffer*>(ibo)->id );
  glVertexArrayElementBuffer( global_vao, static_cast<IndexBuffer*>(ibo)->id );
}

void Gapi::draw(unsigned num_indices)
{
#ifdef DEBUG_SHADER_ERRORS
  glValidateProgram( static_cast<ShaderProgram*>(s)->id );
  GLchar infolog[INFOLOG_SIZE];
  infolog[0] = '\0';
  glGetProgramInfoLog( static_cast<ShaderProgram*>(s)->id, INFOLOG_SIZE, 0, infolog );
  cerr << infolog << endl;
#endif

  glDrawElements( GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, 0 );
}
