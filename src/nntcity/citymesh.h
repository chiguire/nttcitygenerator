
#include <vector>

namespace octet {

	class CityMesh {
		mesh c_mesh;
		material mat;

	public:
		CityMesh() {}

		//
		// LORENZO:
		// takes the streetsList and createse a unique mesh
		// as a collection of parallelepipeds
		//
		// still have to work on the rendering part 
		// and on the material rendering
		//
		void init(std::vector<StreetSides> *streetsList) {
			int num_vertices = streetsList->size()*2;

			mesh_builder b;

			b.init(num_vertices*4, num_vertices*6);
			
			for (int i=0; i<streetsList->size(); i++) {
				vec4 v1 = streetsList->at(i).points[0];
				vec4 v2 = streetsList->at(i).points[1];

				float points_distance = v1.z() - v2.z();
				b.scale( 1.0f, 1.0f, points_distance);
				
				b.translate( 0.0f, 0.0f, points_distance);
				b.add_cube(1.0f);
			}

			b.get_mesh(c_mesh);

		//	mat->make_color(vec4( 1.0f, 0.0f, 0.0f, 1.0f), false, false);
		}

		// just a try on how to draw to boxes as the same mesh
		void debug_createSimpleMesh() {
			mesh_builder b;
			b.init(6*4, 4*6);
			b.scale(1.0f, 1.0f, 5.0f);
			b.add_cube(1.0f);
			b.translate(5.0f, 0.0f, 0.0f);
			b.add_cube(1.0f);
			b.get_mesh(c_mesh);

      mat.make_color(vec4(1, 0, 0, 1), false, false);
		}

		void debugRender(bump_shader &shader, const mat4t &modelToProjection, const mat4t &modelToCamera, vec4 *light_uniforms, const int num_light_uniforms, const int num_lights) {
      mat.render(shader, modelToProjection, modelToCamera, light_uniforms, num_light_uniforms, num_lights);
			c_mesh.render();
		}

	};

  class CompassCard {
    color_shader *cshader;

  public:
    void init(color_shader *cshader_) {
      cshader = cshader_;
    }

    void render(vec3 *camera_position, vec3 *camera_rotation) {
      mat4t modelToWorld;
      modelToWorld.loadIdentity();

      mat4t cameraToWorld;
      cameraToWorld.loadIdentity();
      cameraToWorld.rotate((*camera_rotation)[1], 0.0f, 1.0f, 0.0f);
      cameraToWorld.rotate(-(*camera_rotation)[0], 1.0f, 0.0f, 0.0f);
      cameraToWorld.translate(0.0f, 0.0f, camera_position->z());

      mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld, 0.1f, 1000.0f, 0.08f, 0.08f);

      float x_arrow[] = {
        -1.0f, 0.0f, 0.0f,
         1.0f, 0.0f, 0.0f,
         0.8f, 0.2f, 0.0f,
         1.0f, 0.0f, 0.0f,
         0.8f, -0.2f, 0.0f,
         1.0f, 0.0f, 0.0f
      };

      float y_arrow[] = {
        0.0f, -1.0f, 0.0f,
        0.0f,  1.0f, 0.0f,
        0.2f,  0.8f, 0.0f,
        0.0f,  1.0f, 0.0f,
        -0.2f, 0.8f, 0.0f,
        0.0f,  1.0f, 0.0f
      };

      float z_arrow[] = {
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, 1.0f,
        0.2f, 0.0f, 0.8f,
        0.0f, 0.0f, 1.0f,
        -0.2f, 0.0f, 0.8f,
        0.0f, 0.0f, 1.0f
      };

      glDisable(GL_DEPTH_TEST);
      glEnableVertexAttribArray(attribute_pos);

      cshader->render(modelToProjection, vec4(0.0f, 0.0f, 1.0f, 1.0f));
      glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 0, x_arrow);
      glDrawArrays(GL_LINES, 0, 6);

      cshader->render(modelToProjection, vec4(0.0f, 1.0f, 0.0f, 1.0f));
      glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 0, y_arrow);
      glDrawArrays(GL_LINES, 0, 6);

      cshader->render(modelToProjection, vec4(1.0f, 0.0f, 0.0f, 1.0f));
      glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 0, z_arrow);
      glDrawArrays(GL_LINES, 0, 6);

      glDisableVertexAttribArray(attribute_pos);
      glEnable(GL_DEPTH_TEST);
    }
  };
 
}