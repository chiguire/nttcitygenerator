

namespace octet {


  class skybox_shader : public shader {
    // indices to use with glUniform*()

    // index for model space to projection space matrix
    GLuint modelToProjection_index; 
    GLuint modelToCamera_index;

    GLuint light_uniforms_index;
    GLuint num_lights_index;

    GLuint cubeMap_sampler_index;


  public:
    void init() {

      // vertex shader
      const char vertex_shader[] = SHADER_STR(
        // setting a variable to varying permits to share among vertex shader and fragment shader
        varying vec4 pos_;
      varying vec4 color_;
      varying vec3 norm_;
      varying vec2 uv_;


      // attributes are passed with OPENGL glVertexAttribPointer
      attribute vec4 pos;
      attribute vec3 normal;
      attribute vec4 color;
      attribute vec2 uv;

      // from glUniform
      uniform mat4 modelToProjection;
      uniform mat4 modelToCamera;

      void main() {
        pos_ = vec4(pos.x,-pos.y,pos.z,pos.w);          
        color_ = color;
        uv_ = uv;

        norm_ = (modelToCamera * vec4(normal,0.0)).xyz;
        gl_Position = modelToProjection * pos;
      }
      );

      // fragment shader
      const char fragment_shader[] = SHADER_STR(

        varying vec4 color_;
      varying vec4 pos_;
      varying vec3 norm_;
      varying vec2 uv_;

      uniform samplerCube sampler;


      void main() {
        gl_FragColor = textureCube(sampler, pos_.xyz);
      }
      );

      init_uniforms(vertex_shader, fragment_shader);
    }

    void init_uniforms(const char *vertex_shader, const char *fragment_shader) {
      // use the common shader code to compile and link the shaders
      // the result is a shader program
      shader::init(vertex_shader, fragment_shader);

      // extract the indices of the uniforms to use later
      modelToProjection_index = glGetUniformLocation(program(), "modelToProjection");
      modelToCamera_index = glGetUniformLocation(program(), "modelToCamera");

      cubeMap_sampler_index = glGetUniformLocation(program(), "sampler");

    }

    void render(const mat4t &modelToProjection, const mat4t &modelToCamera, int sampler) {
      // tell openGL to use the program
      shader::render();

      // set the uniforms
      glUniformMatrix4fv(modelToProjection_index, 1, GL_FALSE, modelToProjection.get()); 
      glUniformMatrix4fv(modelToCamera_index, 1, GL_FALSE, modelToCamera.get());

      glUniform1i(cubeMap_sampler_index, sampler);
    }
  };
}
