////////////////////////////////////////////////////////////////////////////////
//
// (C) Dimitri Alvarez, Bogdan Catana, Lorenzo Ciciani, Ciro Duran, Mert Oyman 2013
//
// Procedural Generation for a City Scene
//

namespace octet {
  class engine : public app {
    typedef mat4t mat4t;
    typedef vec4 vec4;
    typedef animation animation;
    typedef scene_node scene_node;

    vec3 camera_position;
    vec3 camera_rotation;

    // named resources loaded from collada file
    resources dict;

    // shaders to draw triangles
    bump_shader object_shader;
    color_shader cshader;

    vec4 light_uniforms_array[5];
    int num_light_uniforms;
    int num_lights;

    // helper to rotate camera about scene
    //mouse_ball ball;

    // helper for picking objects on the screen
    //object_picker picker;

    mat4t cameraToWorld;

    City *city;

    // city mesh obj. working on
    CityMesh *city_mesh;

    CompassCard compassCard;

    int depth;

  public:
    // this is called when we construct the class
    engine(int argc, char **argv) 
    : app(argc, argv)
    //, ball()
    , camera_position(0.0f, 0.0f, 10.0f)
    , camera_rotation(45.0f, 0.0f, 0.0f)
    , cameraToWorld()
    {
    }

    // this is called once OpenGL is initialized
    void app_init() {
      // Shader Set Up
      object_shader.init(false);
      cshader.init();
      compassCard.init(&cshader);

      // Light Set Up
      memset(light_uniforms_array, 0, sizeof(light_uniforms_array));
      light_uniforms_array[0] = vec4(0.3f, 0.3f, 0.3f, 50.0f);
      light_uniforms_array[2] = vec4(0.707f, 0.707f, 0.707f, 0.0f);
      light_uniforms_array[3] = vec4(1.0f, 1.0f, 1.0f, 1.0f);
      light_uniforms_array[4] = vec4(1.0f, 0.0f, 0.0f, 1.0f);
      num_light_uniforms = 5;
      num_lights = 1;

      // Binary Space Partition
      depth = 4;
      //city = City::createFromRectangle(7.0f, 5.0f);
      city = new City();
      vec4 vertices[] = {
        vec4(-5.0f, 0.0f, -8.0f, 1.0f),
        vec4(-6.5f, 0.0f, 9.0f, 1.0f),
        vec4(4.5f, 0.0f, 8.0f, 1.0f),
        vec4(4.0f, 0.0f, -5.0f, 1.0f)
      };
      city->init(vertices);
      city->stepPartition(depth);
      city->printStreets();

      city->calculateIntersections();
      city->printIntersections();

      //
      // city_mesh declaration
      // city_mesh initialization
      //
      city_mesh = new CityMesh();
      dynarray<StreetSides> *streetList = &city->streetsList;

      vec4 dimensions;
      vec4 center;

      city->getDimensions(dimensions);
      city->getCenter(center);

      city_mesh->init(streetList, dimensions, center);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      //picker.init(this);
    }


    void keyboardInput() 
    {
      if (is_key_down('W')) {
        camera_position[1] -= 0.5f * (camera_position[2]/5.0f);
      } else if (is_key_down('S')) {
        camera_position[1] += 0.5f * (camera_position[2]/5.0f);
      }

      if (is_key_down('A')) {
        camera_position[0] -= 0.5f * (camera_position[2]/5.0f);
      } else if (is_key_down('D')) {
        camera_position[0] += 0.5f * (camera_position[2]/5.0f);
      }

      if (is_key_down('Q')) {
        camera_position[2] -= 0.25f;
        if (camera_position[2] < 0.5f) camera_position[2] = 0.5f;
      } else if (is_key_down('E')) {
        camera_position[2] += 0.25f;
      }

      if (is_key_down('F')) {
        camera_rotation[1] += 5.0f;
        if (camera_rotation[1] >= 360.0f) camera_rotation[1] -= 360.0f;
      } else if (is_key_down('H')) {
        camera_rotation[1] -= 5.0f;
        if (camera_rotation[1] < 0.0f) camera_rotation[1] += 360.0f;
      }

      if (is_key_down('G')) {
        camera_rotation[0] -= 5.0f;
        if (camera_rotation[0] < -90.0f) camera_rotation[0] = -90.0f;
      } else if (is_key_down('T')) {
        camera_rotation[0] += 5.0f;
        if (camera_rotation[0] > 90.0f) camera_rotation[0] = 90.0f;
      }
    }


    // this is called to draw the world
    void draw_world(int x, int y, int w, int h) {

      keyboardInput();

      // set a viewport - includes whole window area
      glViewport(x, y, w, h);

      // clear the background to black
      glClearColor(0.5f, 0.5f, 0.5f, 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      /* Set and prepare the shader */
      glEnable(GL_DEPTH_TEST);
      glEnable(GL_BLEND);

      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);

      mat4t modelToWorld;
      modelToWorld.loadIdentity();

      cameraToWorld.loadIdentity();
      cameraToWorld.translate(camera_position.x(), 0.0f, camera_position.y());
      cameraToWorld.rotate(camera_rotation[1], 0.0f, 1.0f, 0.0f);
      cameraToWorld.rotate(-camera_rotation[0], 1.0f, 0.0f, 0.0f);
      cameraToWorld.translate(0.0f, 0.0f, camera_position.z());

      mat4t worldToCamera;
      cameraToWorld.invertQuick(worldToCamera);
      
      mat4t modelToCamera = modelToWorld * worldToCamera;

      mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

      light_uniforms_array[2] = vec4(0.707f, 0.707f, 0.707f, 0.0f) * worldToCamera;

      //ball.update(cameraToWorld);
      //picker.update(app_scene);
    
      //
      // city_mesh render - not working for now
      //
      city_mesh->debugRender(object_shader, modelToProjection, modelToCamera, light_uniforms_array, num_light_uniforms, num_lights);

      //Unbind vertex buffers so normal vertex arrays can work
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

      //city->debugRender(&cshader, &cameraToWorld, float(vx)/float(vy), depth);

      //compassCard.render(&camera_position, &camera_rotation);

    }

  };
}
