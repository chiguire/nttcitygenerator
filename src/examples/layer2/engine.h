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

    vec4 camera_position;
    vec3 camera_rotation;

    // named resources loaded from collada file
    resources dict;

    // shaders to draw triangles
    city_bump_shader city_bump_shader_;
	  city_buildings_bump_shader city_buildings_bump_shader_; 
    bump_shader object_shader;
    color_shader cshader;

    vec4 light_uniforms_array[5];
    int num_light_uniforms;
    int num_lights;
    int  mouse_x, mouse_y;
    int prev_x, prev_y;

    vec3 light_rotation;

    // helper for picking objects on the screen
    //object_picker picker;

    mat4t cameraToWorld;

    City *city;

    // city mesh obj. working on
    CityMesh *city_mesh;

    dynarray<Street> *streetList;
    dynarray<BuildingArea> *buildingAreaList;

    CompassCard compassCard;

    int depth;

    text_overlay textOverlay;

    static const int DRAW_TERRAIN = 0x1;
    static const int DRAW_WATER = 0x2;
    static const int DRAW_ROADS = 0x4;
    static const int DRAW_BUILDINGS = 0x8;
    static const int DRAW_HELP = 0x10;
    static const int DRAW_COMPASS = 0x20;
    static const int DRAW_TERRAIN_NORMALS = 0x40;
    static const int DRAW_ROADS_NORMALS = 0x80;
    static const int DRAW_TERRAIN_WIREFRAME = 0x100;
    static const int DRAW_ROADS_WIREFRAME = 0x200;
    static const int DRAW_BUILDINGS_WIREFRAME = 0x400;

    int drawFlags;

  public:
    // this is called when we construct the class
    engine(int argc, char **argv) 
    : app(argc, argv)
    , textOverlay()
    , camera_position(0.0f, 0.0f, 10.0f, 0.0f)
    , camera_rotation(45.0f, 0.0f, 0.0f)
    , cameraToWorld()
    , light_rotation(45.0f, 30.0f, 0.0f) 
    , drawFlags(DRAW_TERRAIN | DRAW_WATER | DRAW_ROADS | DRAW_BUILDINGS | DRAW_HELP | DRAW_COMPASS
     /* | DRAW_TERRAIN_NORMALS | DRAW_ROADS_NORMALS | DRAW_TERRAIN_WIREFRAME | DRAW_ROADS_WIREFRAME | DRAW_BUILDINGS_WIREFRAME*/ )
    {
    }

    // this is called once OpenGL is initialized
    void app_init() {
      // Shader Set Up
      //city_bump_shader_.init(); // false is default
      object_shader.init(false);
	    city_buildings_bump_shader_.init();
  
      cshader.init();
      compassCard.init(&cshader);

      // Light Set Up
      memset(light_uniforms_array, 0, sizeof(light_uniforms_array));
      light_uniforms_array[0] = vec4(0.1f, 0.1f, 0.1f, 50.0f);
      light_uniforms_array[2] = vec4(sin(light_rotation[0]*3.1415926f/180.0f), sin(light_rotation[1]*3.1415926f/180.0f), cos(light_rotation[0]*3.1415926f/180.0f), 0.0f);
      light_uniforms_array[3] = vec4(1.0f, 1.0f, 1.0f, 1.0f);
      light_uniforms_array[4] = vec4(1.0f, 0.0f, 0.0f, 1.0f);
      num_light_uniforms = 5;
      num_lights = 1;

      // Binary Space Partition
      depth = 8;

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
      //city->printIntersections();
      //city->calculateIntersectionsSpace();
      city->calculateMeshesIntersections();
      //city->printMeshesPoints();
      //
      // city_mesh declaration
      // city_mesh initialization
      //
      city_mesh = new CityMesh();
      streetList = &city->streetsList;

      city->calculateBuildingsAreas(0.9);
      buildingAreaList = &city->buildingAreaList;
      vec4 dimensions;
      vec4 center;

      city->getDimensions(dimensions);
      city->getCenter(center);

      city_mesh->init(streetList, buildingAreaList, dimensions, center);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      textOverlay.init();
    }


    void keyboardInput() 
    {
      static bool justPressed = false;
      vec4 direction(0.0f);
      
      if (!is_key_down(key_alt)) {
        if (is_key_down('W')) {
          direction[1] = -0.25f * (camera_position[2]/5.0f);
        } else if (is_key_down('S')) {
          direction[1] = 0.25f * (camera_position[2]/5.0f);
        }

        if (is_key_down('A')) {
          direction[0] = -0.25f * (camera_position[2]/5.0f);
        } else if (is_key_down('D')) {
          direction[0] = 0.25f * (camera_position[2]/5.0f);
        }

        mat4t directionMatrix(1.0f);
        directionMatrix.rotate(-camera_rotation[1], 0.0f, 0.0f, 1.0f);
        direction = direction * directionMatrix;

        camera_position[0] += direction[0];
        camera_position[1] += direction[1];
      } else {
        if (is_key_down('A')) {
          camera_rotation[1] += 5.0f;
          if (camera_rotation[1] >= 360.0f) camera_rotation[1] -= 360.0f;
        } else if (is_key_down('D')) {
          camera_rotation[1] -= 5.0f;
          if (camera_rotation[1] < 0.0f) camera_rotation[1] += 360.0f;
        }

        if (is_key_down('W')) {
          camera_rotation[0] -= 5.0f;
          if (camera_rotation[0] < -90.0f) camera_rotation[0] = -90.0f;
        } else if (is_key_down('S')) {
          camera_rotation[0] += 5.0f;
          if (camera_rotation[0] > 90.0f) camera_rotation[0] = 90.0f;
        }
      }
      
      if (is_key_down('Q')) {
        camera_position[2] -= 0.25f;
        if (camera_position[2] < 0.5f) camera_position[2] = 0.5f;
      } else if (is_key_down('E')) {
        camera_position[2] += 0.25f;
      }

      if (is_key_down('R')) {
        camera_position[3] += 0.25f * (camera_position[2]/5.0f);
      } else if (is_key_down('Y')) {
        camera_position[3] -= 0.25f * (camera_position[2]/5.0f);
      }

      if (is_key_down('J')) {
        light_rotation[0] += 5.0f;
        if (light_rotation[0] >= 360.0f) light_rotation[0] -= 360.0f;
      } else if (is_key_down('L')) {
        light_rotation[0] -= 5.0f;
        if (light_rotation[0] < 0.0f) light_rotation[0] += 360.0f;
      }

      if (is_key_down('I')) {
        light_rotation[1] -= 5.0f;
        if (light_rotation[1] < -90.0f) light_rotation[1] = -90.0f;
      } else if (is_key_down('K')) {
        light_rotation[1] += 5.0f;
        if (light_rotation[1] > 90.0f) light_rotation[1] = 90.0f;
      }
      
      if (is_key_down(key_alt)) {
        if (is_key_down('Z') && !justPressed) {
          if (drawFlags & DRAW_TERRAIN_NORMALS) {
            drawFlags = drawFlags & ~DRAW_TERRAIN_NORMALS;
          } else {
            drawFlags = drawFlags | DRAW_TERRAIN_NORMALS;
          }
          justPressed = true;
        } else if (!is_key_down('Z')) {
          justPressed = false;
        }

        if (is_key_down('X') && !justPressed) {
          if (drawFlags & DRAW_ROADS_NORMALS) {
            drawFlags = drawFlags & ~DRAW_ROADS_NORMALS;
          } else {
            drawFlags = drawFlags | DRAW_ROADS_NORMALS;
          }
          justPressed = true;
        } else if (!is_key_down('X')) {
          justPressed = false;
        }

        if (is_key_down('C') && !justPressed) {
          if (drawFlags & DRAW_TERRAIN_WIREFRAME) {
            drawFlags = drawFlags & ~DRAW_TERRAIN_WIREFRAME;
            city_mesh->set_mode(drawFlags, buildingAreaList);
          } else {
            drawFlags = drawFlags | DRAW_TERRAIN_WIREFRAME;
            city_mesh->set_mode(drawFlags, buildingAreaList);
          }
          justPressed = true;
        } else if (!is_key_down('C')) {
          justPressed = false;
        }

        if (is_key_down('V') && !justPressed) {
          if (drawFlags & DRAW_ROADS_WIREFRAME) {
            drawFlags = drawFlags & ~DRAW_ROADS_WIREFRAME;
            city_mesh->set_mode(drawFlags, buildingAreaList);
          } else {
            drawFlags = drawFlags | DRAW_ROADS_WIREFRAME;
            city_mesh->set_mode(drawFlags, buildingAreaList);
          }
          justPressed = true;
        } else if (!is_key_down('V')) {
          justPressed = false;
        }

        if (is_key_down('B') && !justPressed) {
          if (drawFlags & DRAW_BUILDINGS_WIREFRAME) {
            drawFlags = drawFlags & ~DRAW_BUILDINGS_WIREFRAME;
            city_mesh->set_mode(drawFlags, buildingAreaList);
          } else {
            drawFlags = drawFlags | DRAW_BUILDINGS_WIREFRAME;
            city_mesh->set_mode(drawFlags, buildingAreaList);
          }
          justPressed = true;
        } else if (!is_key_down('B')) {
          justPressed = false;
        }
      } else {
        if (is_key_down('Z') && !justPressed) {
          if (drawFlags & DRAW_TERRAIN) {
            drawFlags = drawFlags & ~DRAW_TERRAIN;
          } else {
            drawFlags = drawFlags | DRAW_TERRAIN;
          }
          justPressed = true;
        } else if (!is_key_down('Z')) {
          justPressed = false;
        }

        if (is_key_down('X') && !justPressed) {
          if (drawFlags & DRAW_WATER) {
            drawFlags = drawFlags & ~DRAW_WATER;
          } else {
            drawFlags = drawFlags | DRAW_WATER;
          }
          justPressed = true;
        } else if (!is_key_down('X')) {
          justPressed = false;
        }

        if (is_key_down('C') && !justPressed) {
          if (drawFlags & DRAW_ROADS) {
            drawFlags = drawFlags & ~DRAW_ROADS;
          } else {
            drawFlags = drawFlags | DRAW_ROADS;
          }
          justPressed = true;
        } else if (!is_key_down('C')) {
          justPressed = false;
        }

        if (is_key_down('V') && !justPressed) {
          if (drawFlags & DRAW_BUILDINGS) {
            drawFlags = drawFlags & ~DRAW_BUILDINGS;
          } else {
            drawFlags = drawFlags | DRAW_BUILDINGS;
          }
          justPressed = true;
        } else if (!is_key_down('V')) {
          justPressed = false;
        }

        if (is_key_down('B') && !justPressed) {
          if (drawFlags & DRAW_HELP) {
            drawFlags = drawFlags & ~DRAW_HELP;
          } else {
            drawFlags = drawFlags | DRAW_HELP;
          }
          justPressed = true;
        } else if (!is_key_down('B')) {
          justPressed = false;
        }
      
        if (is_key_down('N') && !justPressed) {
          if (drawFlags & DRAW_COMPASS) {
            drawFlags = drawFlags & ~DRAW_COMPASS;
          } else {
            drawFlags = drawFlags | DRAW_COMPASS;
          }
          justPressed = true;
        } else if (!is_key_down('N')) {
          justPressed = false;
        }
      }
      
      if(is_key_down(key_space)){
        this->camera_position = vec4(0.0f, 0.0f, 10.0f, 0.0f);
        this->camera_rotation = vec3(45.0f, 0.0f, 0.0f);
      }
    }
    
    void mouseMovement()
    {
      bool is_mouse_down = is_key_down(key_lmb) && is_key_down(key_alt);
      if (!is_mouse_down) return;
      if (prev_x < mouse_x)
      {
        camera_rotation[1] -= 4.0f;
        prev_x = mouse_x;

        if (camera_rotation[1] < 0.0f)
          camera_rotation[1] += 360.0f;

      }
      else if (prev_x > mouse_x)
      {
        camera_rotation[1] += 4.0f;
        prev_x = mouse_x;

        if (camera_rotation[1] >= 360.0f)
          camera_rotation[1] -= 360.0f;
      }

      if (prev_y < mouse_y)
      {
        camera_rotation[0] += 4.0f;
        prev_y = mouse_y;

        if (camera_rotation[0] > 90.0f)
          camera_rotation[0] = 90.0f;
      }
      else if (prev_y > mouse_y)
      {
        camera_rotation[0] -= 4.0f;
        prev_y = mouse_y;

        if (camera_rotation[0] < -90.0f)
          camera_rotation[0] = -90.0f;
      }

    }


    // this is called to draw the world
    void draw_world(int x, int y, int w, int h) {

      keyboardInput();
      mouseMovement();

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

      setCamara();

      get_mouse_pos(mouse_x, mouse_y);

      mat4t worldToCamera;
      cameraToWorld.invertQuick(worldToCamera);
      
      mat4t modelToCamera = modelToWorld * worldToCamera;

      mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld, 0.1f, 1000.0f, 0.0f, 0.0f, 0.1f*vy/float(vx));

      light_uniforms_array[2] = vec4(sin(light_rotation[0]*3.1415926f/180.0f), sin(light_rotation[1]*3.1415926f/180.0f), cos(light_rotation[0]*3.1415926f/180.0f), 0.0f) * worldToCamera;

      city_mesh->debugRender(object_shader, city_buildings_bump_shader_, cshader, modelToProjection, modelToCamera, light_uniforms_array, num_light_uniforms, num_lights, buildingAreaList, drawFlags);
      //city_mesh->debugRender_newShader(streetList, city_bump_shader_, object_shader, modelToProjection, modelToCamera, light_uniforms_array, num_light_uniforms, num_lights);
      //city->debugRender(&cshader, &cameraToWorld, float(vx)/float(vy), depth);

      if (drawFlags & DRAW_HELP) {
        textOverlay.render(object_shader, object_shader, vx, vy, 0);
      }

      //Unbind vertex buffers so normal vertex arrays can work
      if (drawFlags & DRAW_COMPASS) {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        compassCard.render(&camera_position, &camera_rotation, float(vy)/float(vx));
      }

    }

    void setCamara() 
    {
      cameraToWorld.loadIdentity();
      cameraToWorld.translate(camera_position.x(), camera_position.w(), camera_position.y());
      cameraToWorld.rotate(camera_rotation[1], 0.0f, 1.0f, 0.0f);
      cameraToWorld.rotate(-camera_rotation[0], 1.0f, 0.0f, 0.0f);
      cameraToWorld.translate(0.0f, 0.0f, camera_position.z());
    }

  };
}
