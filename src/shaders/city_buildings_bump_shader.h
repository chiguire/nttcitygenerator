

namespace octet {
  class city_buildings_bump_shader : public shader {
    // indices to use with glUniform*()

    GLuint modelToProjection_index; // index for model space to projection space matrix
    GLuint modelToCamera_index;     // second matrix used for lighting maps model to camera space
    GLuint cameraToProjection_index;// used in skinned shader
    GLuint light_uniforms_index;    // lighting parameters for fragment shader
    GLuint num_lights_index;        // how many lights?
    GLuint samplers_index;          // index for texture samplers
	GLuint bulding_height_index;		// index for building height 
	GLuint building_area_index;			// index for building area
	GLuint texture_switcher_index; 
	GLuint part_to_render_index;

    void init_uniforms(const char *vertex_shader, const char *fragment_shader) {
      // use the common shader code to compile and link the shaders
      // the result is a shader program
      shader::init(vertex_shader, fragment_shader);

      // extract the indices of the uniforms to use later
      modelToProjection_index	= glGetUniformLocation(program(), "modelToProjection");
      cameraToProjection_index	= glGetUniformLocation(program(), "cameraToProjection");
      modelToCamera_index		= glGetUniformLocation(program(), "modelToCamera");
      light_uniforms_index		= glGetUniformLocation(program(), "light_uniforms");
      num_lights_index			= glGetUniformLocation(program(), "num_lights");
      samplers_index			= glGetUniformLocation(program(), "samplers");
	  bulding_height_index		= glGetUniformLocation(program(), "b_height"); 
	  building_area_index		= glGetUniformLocation(program(), "b_area"); 
	  texture_switcher_index	= glGetUniformLocation(program(), "switcher"); 
	  part_to_render_index		= glGetUniformLocation(program(), "part_to_render"); 
    }

  public:
    void init(bool is_skinned=false) {
      // this is the vertex shader for regular geometry
      // it is called for each corner of each triangle
      // it inputs pos and uv from each corner
      // it outputs gl_Position, normal_ and uv_ to the rasterizer
      // normal_ is the normal in camera space to make calculations easier
      const char vertex_shader[] = SHADER_STR(
        varying vec2 uv_;
        varying vec3 normal_;
        varying vec3 tangent_;
        varying vec3 bitangent_;
		varying vec3 normal_t_;
      
        attribute vec4 pos;
        attribute vec3 normal;
        attribute vec3 tangent;
        attribute vec3 bitangent;
        attribute vec2 uv;
      
        uniform mat4 modelToProjection;
        uniform mat4 modelToCamera;
      
        void main() {
          uv_ = uv;
          normal_ = (modelToCamera * vec4(normal,0)).xyz;
		  normal_t_ = normal;
          tangent_ = (modelToCamera * vec4(tangent,0)).xyz;
          bitangent_ = (modelToCamera * vec4(bitangent,0)).xyz;
          gl_Position = modelToProjection * pos;
        }
      );

      // this is the vertex shader for skinned geometry
      // this is the shader for skinned geometry
      // it is not terribly efficient, but does the job.
      // it is generally better to use quaternions as they
      // are smaller and don't collapse joints.
      const char skinned_vertex_shader[] = SHADER_STR(
        varying vec2 uv_;
        varying vec3 normal_;
        varying vec3 tangent_;
        varying vec3 bitangent_;
      
        attribute vec4 pos;
        attribute vec3 normal;
        attribute vec3 tangent;
        attribute vec3 bitangent;
        attribute vec2 uv;
        attribute vec3 blendweight;
        attribute vec4 blendindices;
      
        uniform mat4 cameraToProjection;
        uniform mat4 modelToCamera[192];
      
        void main() {
          uv_ = uv;
          ivec4 index = ivec4(blendindices);
          mat4 m2c0 = modelToCamera[index.x];
          mat4 m2c1 = modelToCamera[index.y];
          mat4 m2c2 = modelToCamera[index.z];
          mat4 m2c3 = modelToCamera[index.w];
          float blend0 = 1.0 - blendweight.x - blendweight.y - blendweight.z;
          mat4 blendedModelToCamera = m2c0 * blend0 + m2c1 * blendweight.x + m2c2 * blendweight.y + m2c3 * blendweight.z;
          normal_ = normalize((blendedModelToCamera * vec4(normal,0)).xyz);
          tangent_ = normalize((blendedModelToCamera * vec4(tangent,0)).xyz);
          bitangent_ = normalize((blendedModelToCamera * vec4(bitangent,0)).xyz);
          gl_Position = cameraToProjection * (blendedModelToCamera * pos);
        }
      );

      // this is the fragment shader
      // after the rasterizer breaks the triangle into fragments
      // this is called for every fragment
      // it outputs gl_FragColor, the color of the pixel and inputs normal_ and uv_
      // the four samplers give emissive, diffuse, specular and ambient colors
      const char fragment_shader[] = SHADER_STR(
        const int max_lights = 4;
        varying vec2 uv_;
        varying vec3 normal_;
		varying vec3 normal_t_;
        varying vec3 tangent_;
        varying vec3 bitangent_;

        uniform vec4 light_uniforms[1+max_lights*4];
		uniform int num_lights;

		uniform sampler2D samplers[9];
        
		uniform float b_height; 
		uniform float b_area;
		uniform int switcher;
		uniform int part_to_render;

		
		vec4 texture_selector() {
			vec4 color; 
		  if (switcher == 0) {
			  if ( part_to_render == 1 ) {
				  //color = texture2D(samplers[0], uv_);
				  color = vec4(0.2, 0.2, 0.2, 1.0);
			  } else if (part_to_render == 2) {
				  color = texture2D(samplers[0], uv_);

			  } else {

				 if (b_area < 0.4) {
					 color = texture2D(samplers[8], vec2(uv_.x, uv_.y*3.0));

				 } else if (b_area > 50.0) {

					  if (b_height > 4.5) {
						 color = texture2D(samplers[8], vec2(uv_.x*5.0, uv_.y*5.0));
					 } else if (b_height > 3.0) {
						color = texture2D(samplers[7], vec2(uv_.x*5.0, uv_.y*3.0));
					 } else if (b_height > 1.0) {
						 color = texture2D(samplers[6], vec2(uv_.x*3.0, uv_.y*10.0));
					 } else {
						color = vec4(0.2, 0.2, 0.2, 1.0); 					
					 }

				 } else  {
					 if (b_height > 4.5) {
						 color = texture2D(samplers[8], uv_);
					 } else if (b_height > 3.0) {
						color = texture2D(samplers[7], uv_);
					 } else if (b_height > 1.0) {
						 color = texture2D(samplers[6], uv_*2.0);
					 } else {
						color = vec4(0.2, 0.2, 0.2, 1.0); 					
					 }
				 }
			  }
		  } else {
			  if (part_to_render == 1) {
					  color = vec4(0.2, 0.2, 0.2, 1.0);
			  } else {

				 if (b_area < 0.4) {
					 
					 color = vec4(0.2, 0.2, 0.8, 1.0); 
				 } else {
					 if (b_height > 4.0) {
						
						color = vec4(1.0, 0.2, 0.2, 1.0);
					 } else if (b_height > 3.0) {
						
							color = vec4(0.2, 1.0, 0.2, 1.0);
					 } else if (b_height > 1.0) {
						 
							color = vec4(0.2, 0.2, 1.0, 1.0);
					 } else {
						
						  color = vec4(1.0, 1.0, 0.2, 1.0); 					
					 }
				 }
			  }
		  }

			  return color; 
		}

      

        void main() {
          float shininess = texture2D(samplers[5], uv_).x * 255.0;
          vec3 bump = normalize(vec3(texture2D(samplers[4], uv_).xy-vec2(0.5, 0.5), 1));
          vec3 nnormal = normal_; //normalize(bump.x * tangent_ + bump.y * bitangent_ + bump.z * normal_);
          vec3 diffuse_light = vec3(0.3, 0.3, 0.3);
          vec3 specular_light = vec3(0, 0, 0);

          for (int i = 0; i != num_lights; ++i) {
            vec3 light_direction = light_uniforms[i * 4 + 2].xyz;
            vec3 light_color = light_uniforms[i * 4 + 3].xyz;
            vec3 half_direction = normalize(light_direction + vec3(0, 0, 1));

            float diffuse_factor = max(dot(light_direction, nnormal), 0.0);
            float specular_factor = pow(max(dot(half_direction, nnormal), 0.0), shininess) * diffuse_factor;

            diffuse_light += diffuse_factor * light_color;
            specular_light += specular_factor * light_color;
          }


		  vec4 ambient;
		  vec4 diffuse; 
		  
		  ambient = diffuse = texture_selector(); 
          
          vec4 emission = texture2D(samplers[2], uv_);
          vec4 specular = texture2D(samplers[3], uv_);

          vec3 ambient_light = light_uniforms[0].xyz;

          gl_FragColor.xyz = 
            ambient_light * ambient.xyz +
            diffuse_light * diffuse.xyz +
            emission.xyz +
            specular_light * specular.xyz
          ;
		  
          gl_FragColor.w = diffuse.w;

        }
      );
    
      // use the common shader code to compile and link the shaders
      // the result is a shader program
      init_uniforms(is_skinned ? skinned_vertex_shader : vertex_shader, fragment_shader);
    }

    void render(const mat4t &modelToProjection, const mat4t &modelToCamera, const vec4 *light_uniforms, int num_light_uniforms, int num_lights, float bulding_height, float building_area, int texture_switcher, int part_to_render) {
      // tell openGL to use the program
      shader::render();

      // customize the program with uniforms
      glUniformMatrix4fv(modelToProjection_index, 1, GL_FALSE, modelToProjection.get());
      glUniformMatrix4fv(modelToCamera_index, 1, GL_FALSE, modelToCamera.get());

      glUniform4fv(light_uniforms_index, num_light_uniforms, (float*)light_uniforms);
      glUniform1i(num_lights_index, num_lights);
	  
	  glUniform1f(bulding_height_index, bulding_height); 
	  glUniform1f(building_area_index, building_area); 
	  glUniform1i(texture_switcher_index, texture_switcher); 
	  glUniform1i(part_to_render_index, part_to_render); 

      // we use textures 0-3 for material properties.
      static const GLint samplers[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
      glUniform1iv(samplers_index, 9, samplers);
    }

    void render_skinned(const mat4t &cameraToProjection, const mat4t *modelToCamera, int num_matrices, const vec4 *light_uniforms, int num_light_uniforms, int num_lights) {
      // tell openGL to use the program
      shader::render();

      // customize the program with uniforms
      glUniformMatrix4fv(cameraToProjection_index, 1, GL_FALSE, cameraToProjection.get());
      glUniformMatrix4fv(modelToCamera_index, num_matrices, GL_FALSE, (float*)modelToCamera);

      glUniform4fv(light_uniforms_index, num_light_uniforms, (float*)light_uniforms);
      glUniform1i(num_lights_index, num_lights);

      // we use textures 0-3 for material properties.
      static const GLint samplers[] = { 0, 1, 2, 3, 4 };
      glUniform1iv(samplers_index, 5, samplers);
    }
  };
}
