////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// bump map example: More advanced shaders
//
// Level: 1
//
// Demonstrates:
//   Basic framework app
//   Shaders
//   Basic Matrices
//   Stock meshes
//   mesh operations
//   Phong bumpmap shader
//

namespace octet {
  class bump_app : public octet::app {
    // Matrix to transform points on our triangle to the world space
    // This allows us to move and rotate our triangle
    mat4t modelToWorld;

    // Matrix to transform points in our camera space to the world.
    // This lets us move our camera
    mat4t cameraToWorld;

    // shader to draw a shaded, textured triangle
    bump_shader bump_shader_;

    // handle for the texture
    mesh cube_mesh;
    //mesh cube_mesh_normals;

    GLuint diffuse;
    GLuint ambient;
    GLuint emission;
    GLuint specular;
    GLuint bump;
  public:

    // this is called when we construct the class
    bump_app(int argc, char **argv) : app(argc, argv) {
    }

    // this is called once OpenGL is initialized
    void app_init() {
      // set up the shader
      bump_shader_.init();

      // set up the matrices with a camera 5 units from the origin
      modelToWorld.loadIdentity();
      cameraToWorld.loadIdentity();
      cameraToWorld.translate(0, 0, 3);

      mesh tmp;
      tmp.make_sphere(2, 64, 64);
      cube_mesh.add_3d_normals(tmp);
      //cube_mesh_normals.make_normal_visualizer(cube_mesh, 0.1f, attribute_bitangent);

      diffuse = ambient = resources::get_texture_handle(GL_RGB, "!bricks");
      emission = resources::get_texture_handle(GL_RGB, "#000000");
      specular = resources::get_texture_handle(GL_RGB, "#ffffff");
      bump = resources::get_texture_handle(GL_RGB, "!bump");
    }

    // this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      // set a viewport - includes whole window area
      glViewport(x, y, w, h);

      // clear the background to black
      glClearColor(0, 0, 0, 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // allow Z buffer depth testing (closer objects are always drawn in front of far ones)
      glEnable(GL_DEPTH_TEST);

      // improve draw speed by culling back faces - and avoid flickering edges
      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);
      glFrontFace(GL_CW);

      // build a projection matrix: model -> world -> camera -> projection
      // the projection space is the cube -1 <= x/w, y/w, z/w <= 1
      mat4t modelToCamera;
      mat4t worldToCamera;
      mat4t modelToProjection = mat4t::build_camera_matrices(modelToCamera, worldToCamera, modelToWorld, cameraToWorld);

      vec4 lights[5];
      memset(lights, 0, sizeof(lights));
      lights[0] = vec4(0.3f, 0.3f, 0.3f, 50);
      lights[2] = vec4(0.707f, 0, 0.707f, 0) * worldToCamera;
      lights[3] = vec4(1, 1, 1, 1);
      lights[4] = vec4(1, 0, 0, 1);

      bump_shader_.render(modelToProjection, modelToCamera, lights, 5, 1);

      // spin the cube by rotating about X, Y and Z
      modelToWorld.rotateY(1);
      //modelToWorld.rotateX(2);
      //modelToWorld.rotateY(1.5f);

      // set textures 0, 1, 2, 3 to their respective values
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, diffuse);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, ambient);
      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, emission);
      glActiveTexture(GL_TEXTURE3);
      glBindTexture(GL_TEXTURE_2D, specular);
      glActiveTexture(GL_TEXTURE4);
      glBindTexture(GL_TEXTURE_2D, bump);
      glActiveTexture(GL_TEXTURE0);

      cube_mesh.render();
      //cube_mesh_normals.render();
    }
  };
}
