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

    // named resources loaded from collada file
    resources dict;

    // shaders to draw triangles
    bump_shader object_shader;
    bump_shader skin_shader;
    color_shader cshader;

    // helper to rotate camera about scene
    mouse_ball ball;

    // helper for picking objects on the screen
    object_picker picker;

    mat4t cameraToWorld;

    City *city;

	// city mesh obj. working on
	CityMesh *city_mesh;

    int depth;

  public:
    // this is called when we construct the class
    engine(int argc, char **argv) 
    : app(argc, argv)
    , ball()
    , cameraToWorld()
    {
    }

    // this is called once OpenGL is initialized
    void app_init() {
      // set up the shaders
      object_shader.init(false);
      skin_shader.init(true);
      cshader.init();

      depth = 5;

      city = City::createFromRectangle(7.0f, 5.0f);
      city->stepPartition(depth);

      city->printStreets();

	  //
	  // city_mesh declaration
	  // city_mesh initialization
	  //
	  city_mesh = new CityMesh();
	  std::vector<StreetSides> *streetList = &city->streetsList;
	  //city_mesh->init(streetList);
	  city_mesh->debug_createSimpleMesh();

      picker.init(this);
    }

    // this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      // set a viewport - includes whole window area
      glViewport(x, y, w, h);

      // clear the background to black
      glClearColor(0.5f, 0.5f, 0.5f, 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // allow Z buffer depth testing (closer objects are always drawn in front of far ones)
      glEnable(GL_DEPTH_TEST);

      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);

      cameraToWorld.loadIdentity();
      cameraToWorld.rotate(45, -1.0f, 0.0f, 0.0f);
      cameraToWorld.translate(0.0f, 2.0f, 8.0f);
      //ball.update(cameraToWorld);
      //picker.update(app_scene);

      city->debugRender(&cshader, &cameraToWorld, float(vx)/float(vy), depth);
	  
	  //
	  // city_mesh render - not working for now
	  //
	   city_mesh->debugRender();
    }
  };
}
