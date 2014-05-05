////////////////////////////////////////////////////////////////////////////////
//
// (C) Dimitri Alvarez, Bogdan Catana, Lorenzo Ciciani, Ciro Duran, Mert Oyman 2013
//
// Procedural Generation for a City Scene
//

namespace octet {
  enum DrawOptions {
    DRAW_TERRAIN = 0x1,
    DRAW_WATER = 0x2,
    DRAW_ROADS = 0x4,
    DRAW_BUILDINGS = 0x8,
    DRAW_HELP = 0x10,
    DRAW_COMPASS = 0x20,
    DRAW_TERRAIN_NORMALS = 0x40,
    DRAW_ROADS_NORMALS = 0x80,
    DRAW_TERRAIN_WIREFRAME = 0x100,
    DRAW_ROADS_WIREFRAME = 0x200,
    DRAW_BUILDINGS_WIREFRAME = 0x400,
  };

  class engine : public app {
    typedef mat4t mat4t;
    typedef vec4 vec4;
    typedef animation animation;
    typedef scene_node scene_node;

    // shaders to draw triangles
    city_bump_shader city_bump_shader_;
    city_buildings_bump_shader city_buildings_bump_shader_; 
    bump_shader object_shader;
    color_shader cshader;
    skybox_shader sb_shader;

    vec4 light_uniforms_array[5];
    int num_light_uniforms;
    int num_lights;
    int  mouse_x, mouse_y;
    int prev_x, prev_y;

    vec3 light_rotation;

    camera_controls cameraControls;
    mat4t cameraToWorld;

    City *city;
    CityMesh *city_mesh;

    dynarray<Street> *streetList;
    dynarray<BuildingArea> *buildingAreaList;

    std::vector <ref<LampModel>> *lampModels;

    CompassCard compassCard;

    int depth;

    text_overlay textOverlay;

    int drawFlags;
	int draw_texture_mode; 

  public:
    // this is called when we construct the class
    engine(int argc, char **argv) 
    : app(argc, argv)
    , textOverlay()
    , cameraControls()
    , cameraToWorld()
    , light_rotation(45.0f, 30.0f, 0.0f) 
    , drawFlags(DRAW_TERRAIN | DRAW_WATER | DRAW_ROADS | DRAW_BUILDINGS | DRAW_HELP | DRAW_COMPASS
     /* | DRAW_TERRAIN_NORMALS | DRAW_ROADS_NORMALS | DRAW_TERRAIN_WIREFRAME | DRAW_ROADS_WIREFRAME | DRAW_BUILDINGS_WIREFRAME*/ )
    {
    }

    // this is called once OpenGL is initialized
    void app_init() {
      // Shader Set Up
      object_shader.init(false);
      city_buildings_bump_shader_.init();
  
      cshader.init();

      sb_shader.init();

      compassCard.init(&cshader);

    draw_texture_mode = 0; 

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
        vec4(-10.0f, 0.0f, -16.0f, 1.0f),
        vec4(-13.0f, 0.0f, 18.0f, 1.0f),
        vec4(9.0f, 0.0f, 16.0f, 1.0f),
        vec4(8.0f, 0.0f, -10.0f, 1.0f)
      }; 

      /*vec4 vertices[] = {
      vec4(-5.0f, 0.0f, -8.0f, 1.0f),
      vec4(-6.5f, 0.0f, 9.0f, 1.0f),
      vec4(4.5f, 0.0f, 8.0f, 1.0f),
      vec4(4.0f, 0.0f, -5.0f, 1.0f)
      };*/ 

      city->init(vertices);
      city->stepPartition(depth);
      // city->printStreets();

      city->calculateIntersections();
      //city->printIntersections();
      //city->calculateIntersectionsSpace();
      city->calculateMeshesIntersections();
      //city->printMeshesPoints();
      city->calculateBuildingsAreas(); 

      //
      // city_mesh declaration
      // city_mesh initialization
      //
      city_mesh = new CityMesh();
      city->setCityMesh(city_mesh);

      city->loadModels();
      city->generateLamps();

      streetList = &city->streetsList;
      
      lampModels = &city->lamps;
      
      //city->calculateBuildingsAreas(0.75);
      buildingAreaList = &city->buildingAreaList;

      vec4 dimensions;
      vec4 center;

      city->getDimensions(dimensions);
      city->getCenter(center);

      city_mesh->init(streetList, buildingAreaList, dimensions, center);

      cameraControls.init(city, city_mesh);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      textOverlay.init();
    }


    void keyboardInput() 
    {
      static bool justPressed = false;
      vec4 direction(0.0f);
      
      if (is_key_down('M') && !justPressed) {
        cameraControls.switchToNextMode();
        printf("Switching to camera mode: %s.\n", cameraControls.getMode() == CAMERAMODE_FREEFORM? "Freeform": "Walkthrough");
        justPressed = true;
      } else if (!is_key_down('M') && justPressed) {
        justPressed = false;
      }
      
      if(is_key_down(key_space)){
        cameraControls.resetCamera();
      }

      if (!is_key_down(key_alt)) {
        if (is_key_down('W')) {
          direction[1] = -1.0f;
        } else if (is_key_down('S')) {
          direction[1] = 1.0f;
        }

        if (is_key_down('A')) {
          direction[0] = -1.0f;
        } else if (is_key_down('D')) {
          direction[0] = 1.0f;
        }

        cameraControls.moveWithCamera(direction);
      } else {
        if (is_key_down('A')) {
          cameraControls.rotateCameraLeft();
        } else if (is_key_down('D')) {
          cameraControls.rotateCameraRight();
        }

        if (is_key_down('W')) {
          cameraControls.rotateCameraUp();
        } else if (is_key_down('S')) {
          cameraControls.rotateCameraDown();
        }
      }
      
      if (is_key_down('Q')) {
        cameraControls.zoomCameraIn();
      } else if (is_key_down('E')) {
        cameraControls.zoomCameraOut();
      }

      if (is_key_down('R')) {
        cameraControls.moveCameraUp(); 
      } else if (is_key_down('Y')) {
        cameraControls.moveCameraDown();
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
        } else if (!is_key_down('Z') && justPressed) {
          justPressed = false;
        }

        if (is_key_down('X') && !justPressed) {
          if (drawFlags & DRAW_ROADS_NORMALS) {
            drawFlags = drawFlags & ~DRAW_ROADS_NORMALS;
          } else {
            drawFlags = drawFlags | DRAW_ROADS_NORMALS;
          }
          justPressed = true;
        } else if (!is_key_down('X') && !justPressed) {
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
        } else if (!is_key_down('C') && !justPressed) {
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
        } else if (!is_key_down('V') && !justPressed) {
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
        } else if (!is_key_down('B') && !justPressed) {
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
        } else if (!is_key_down('Z') && !justPressed) {
          justPressed = false;
        }

        if (is_key_down('X') && !justPressed) {
          if (drawFlags & DRAW_WATER) {
            drawFlags = drawFlags & ~DRAW_WATER;
          } else {
            drawFlags = drawFlags | DRAW_WATER;
          }
          justPressed = true;
        } else if (!is_key_down('X') && !justPressed) {
          justPressed = false;
        }

        if (is_key_down('C') && !justPressed) {
          if (drawFlags & DRAW_ROADS) {
            drawFlags = drawFlags & ~DRAW_ROADS;
          } else {
            drawFlags = drawFlags | DRAW_ROADS;
          }
          justPressed = true;
        } else if (!is_key_down('C') && !justPressed) {
          justPressed = false;
        }

        if (is_key_down('V') && !justPressed) {
          if (drawFlags & DRAW_BUILDINGS) {
            drawFlags = drawFlags & ~DRAW_BUILDINGS;
          } else {
            drawFlags = drawFlags | DRAW_BUILDINGS;
          }
          justPressed = true;
        } else if (!is_key_down('V') && !justPressed) {
          justPressed = false;
        }

        if (is_key_down('B') && !justPressed) {
          if (drawFlags & DRAW_HELP) {
            drawFlags = drawFlags & ~DRAW_HELP;
          } else {
            drawFlags = drawFlags | DRAW_HELP;
          }
          justPressed = true;
        } else if (!is_key_down('B') && !justPressed) {
          justPressed = false;
        }
      
        if (is_key_down('N') && !justPressed) {
          if (drawFlags & DRAW_COMPASS) {
            drawFlags = drawFlags & ~DRAW_COMPASS;
          } else {
            drawFlags = drawFlags | DRAW_COMPASS;
          }
          justPressed = true;
        } else if (!is_key_down('N') && !justPressed) {
          justPressed = false;
        }


		if (is_key_down('T') && !justPressed) {
          if (draw_texture_mode == 0) {
			draw_texture_mode = 1;
          } else {
            draw_texture_mode = 0; 
          }
          justPressed = true;
        } else if (!is_key_down('T') && !justPressed) {
          justPressed = false;
        }
      }
    }
    
    void mouseMovement()
    {
      bool is_mouse_down = is_key_down(key_lmb) && is_key_down(key_alt);
      if (is_mouse_down) {
        cameraControls.startMouseDrag(mouse_x, mouse_y);
        cameraControls.moveMouseDrag(mouse_x, mouse_y);
      } else {
        cameraControls.endMouseDrag();
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

      city_mesh->debugRender(object_shader, city_buildings_bump_shader_, cshader, sb_shader, modelToProjection, modelToCamera, cameraToWorld,light_uniforms_array, num_light_uniforms, num_lights, buildingAreaList, lampModels, drawFlags, draw_texture_mode);
      //city_mesh->debugRender_newShader(streetList, city_bump_shader_, object_shader, modelToProjection, modelToCamera, light_uniforms_array, num_light_uniforms, num_lights);
      //city->debugRender(&cshader, &cameraToWorld, float(vx)/float(vy), depth);

      if (drawFlags & DRAW_HELP) {
        glDisable(GL_DEPTH_TEST);
        textOverlay.render(object_shader, object_shader, vx, vy, 0);
        glEnable(GL_DEPTH_TEST);
      }

      //Unbind vertex buffers so normal vertex arrays can work
      if (drawFlags & DRAW_COMPASS) {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        vec4 light_dir(sin(light_rotation[0]*3.1415926f/180.0f), sin(light_rotation[1]*3.1415926f/180.0f), cos(light_rotation[0]*3.1415926f/180.0f), 1.0f);

        compassCard.render(&cameraControls.getPosition(), &cameraControls.getRotation(), float(vy)/float(vx), light_dir);
      }

    }

    void setCamara() {

      cameraControls.updateCamera();

      cameraToWorld.loadIdentity();
      cameraToWorld.translate(cameraControls.getPosition().x(), cameraControls.getPosition().w(), cameraControls.getPosition().y());
      cameraToWorld.rotate(cameraControls.getRotation().y(), 0.0f, 1.0f, 0.0f);
      cameraToWorld.rotate(-cameraControls.getRotation().x(), 1.0f, 0.0f, 0.0f);
      cameraToWorld.translate(0.0f, 0.0f, cameraControls.getPosition().z());
    }

  };
}
