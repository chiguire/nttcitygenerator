namespace octet {

	class CityMesh {
    dynarray <mesh *> roadMeshes;
    dynarray <mesh *> pavementMeshes;

		material *roadMaterial;
    material *pavementMaterial;

    static dynarray<image *> *imageArray_;

    static dynarray<image *> *getImageArray() {
      if (!imageArray_) {
        imageArray_ = new dynarray<image *>();
        char *files[] = {
          "assets/citytex/pavement.gif",
          "assets/citytex/road.gif",
          0
        };

        for (int i = 0; files[i]; i++) { 
          image *img = new image(files[i]);
          img->load();
          imageArray_->push_back(img);
        }
      }

      return imageArray_;
    }

	public:
		CityMesh() {
      roadMeshes.reset();
      pavementMeshes.reset();
    }

		void init(dynarray<StreetSides> *streetsList) {
			for (int i = 0; i < streetsList->size(); i++) {
    		mesh_builder roadMeshBuilder;
        mesh_builder pavementMeshBuilder;

        // Creating road
		  	roadMeshBuilder.init(0, 0);
				vec4 v1 = (*streetsList)[i].points[0];
				vec4 v2 = (*streetsList)[i].points[1];

        //printf("Adding street %d, from (%.2f, %.2f) to (%.2f, %.2f)\n", (i+1), v1.x(), v1.z(), v2.x(), v2.z());

        float angleY = atan2f(v2.x() - v1.x(), v2.z() - v1.z())*180.0f/3.14159265359f;
        vec4 vMidpoint = vec4(v1.x() + (v2.x()-v1.x())/2.0f, v1.y() + (v2.y()-v1.y())/2.0f, v1.z() + (v2.z()-v1.z())/2.0f, 1.0f);

				float points_distance = (v2 - v1).length();
				roadMeshBuilder.translate(vMidpoint.x(), vMidpoint.y(), vMidpoint.z());
        roadMeshBuilder.rotate(angleY, 0.0f, 1.0f, 0.0f);
        roadMeshBuilder.add_cuboid(0.1f, 0.02f, points_distance/2.0f);

        mesh *m = new mesh();
        roadMeshBuilder.get_mesh(*m);

        roadMeshes.push_back(m);

        //Creating pavements
        //left
        pavementMeshBuilder.init(0, 0);
        pavementMeshBuilder.translate(vMidpoint.x(), vMidpoint.y(), vMidpoint.z());
        pavementMeshBuilder.rotate(angleY, 0.0f, 1.0f, 0.0f);
        pavementMeshBuilder.translate(-0.1f-0.01f, 0.0f, 0.0f);
        pavementMeshBuilder.add_cuboid(0.02f, 0.04f, points_distance/2.0f-0.1f);

        m = new mesh();
        pavementMeshBuilder.get_mesh(*m);
        pavementMeshes.push_back(m);

        //right
        pavementMeshBuilder.init(0, 0);
        pavementMeshBuilder.translate(vMidpoint.x(), vMidpoint.y(), vMidpoint.z());
        pavementMeshBuilder.rotate(angleY, 0.0f, 1.0f, 0.0f);
        pavementMeshBuilder.translate(0.1f+0.01f, 0.0f, 0.0f);
        pavementMeshBuilder.add_cuboid(0.02f, 0.04f, points_distance/2.0f-0.1f);

        m = new mesh();
        pavementMeshBuilder.get_mesh(*m);
        pavementMeshes.push_back(m);
			}


      pavementMaterial = new material((*getImageArray())[0]);
      roadMaterial = new material((*getImageArray())[1]);
		}

		void debugRender(bump_shader &shader, const mat4t &modelToProjection, const mat4t &modelToCamera, vec4 *light_uniforms, const int num_light_uniforms, const int num_lights) {
      roadMaterial->render(shader, modelToProjection, modelToCamera, light_uniforms, num_light_uniforms, num_lights);
      for (auto m = roadMeshes.begin(); m != roadMeshes.end(); m++) {
        (*m)->render();
      }
      pavementMaterial->render(shader, modelToProjection, modelToCamera, light_uniforms, num_light_uniforms, num_lights);
      for (auto m = pavementMeshes.begin(); m != pavementMeshes.end(); m++) {
        (*m)->render();
      }
		}
	};

  dynarray <image *> *CityMesh::imageArray_;

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