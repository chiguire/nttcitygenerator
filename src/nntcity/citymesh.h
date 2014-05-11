namespace octet {

  class CityMesh {

    mesh roadLeftMesh;
    mesh roadRightMesh;
    mesh pavementMesh;
    mesh surfaceMesh;
    mesh waterMesh;

    mesh roadLeftNormalsMesh;
    mesh roadRightNormalsMesh;
    mesh pavementNormalsMesh;
    mesh surfaceNormalsMesh;

    material *roadMaterialLeft;
    material *roadMaterialRight;
    material *pavementMaterial;  
    material *grassMaterial;
    material *waterMaterial;
    material *buldingMaterial;

    material *lampMaterial;
    material *trafficLightMaterial;
    material *hydrantMaterial;
    material *postBoxMaterial;
    material *treeMaterial;
    material *tree2Material;
    material *benchMaterial;
    material *binMaterial;

    HeightMap *heightMap;
    
    static dynarray<image *> *imageArray_;

    mesh skyboxMesh;
    GLuint sky_box_textureObj;
    dynarray<uint8_t> buffer;
    dynarray<uint8_t> images;

    public:

    enum TextureAsset {
      TEXTUREASSET_PAVEMENT,
      TEXTUREASSET_ROADLEFT,
      TEXTUREASSET_ROADRIGHT,
      TEXTUREASSET_HEIGHTMAP,
      TEXTUREASSET_BUILDING,
      TEXTUREASSET_BUILDING_RES_1,
      TEXTUREASSET_BUILDING_RES_2,
      TEXTUREASSET_BUILDING_RES_3,
      TEXTUREASSET_GRASS_DIFFUSE,
      TEXTUREASSET_GRASS_DISP,
      TEXTUREASSET_GRASS_NORMAL,
      TEXTUREASSET_GRASS_DETAIL,
      TEXTUREASSET_WATER_DIFFUSE,
      TEXTUREASSET_WATER_DISP,
      TEXTUREASSET_WATER_NORMAL,
      TEXTUREASSET_LAMP_TEXTURE,
      TEXTUREASSET_TRAFFICLIGHT_TEXTURE,
      TEXTUREASSET_HYDRANT_TEXTURE,
      TEXTUREASSET_POSTBOX_TEXTURE,
      TEXTUREASSET_TREE_TEXTURE,
      TEXTUREASSET_TREE2_TEXTURE,
      TEXTUREASSET_BENCH_TEXTURE,
      TEXTUREASSET_BIN_TEXTURE,
    };

    static dynarray<image *> *getImageArray() {
      if (!imageArray_) {
        imageArray_ = new dynarray<image *>();
        char *files[] = {
          "assets/citytex/pavement.gif",
          "assets/citytex/road_left.gif",
          "assets/citytex/road_right.gif",
          "assets/citytex/heightmap6.gif",
          "assets/citytex/grass/detail.gif",
          "assets/citytex/buildings/building_residential_low.gif",
          "assets/citytex/buildings/building_office_glass.gif",
          "assets/citytex/buildings/building_office_highglass.gif",
          "assets/citytex/grass/06_DIFFUSE.jpg",
          "assets/citytex/grass/06_DISP.jpg",
          "assets/citytex/grass/06_NORMAL.jpg",
          "assets/citytex/grass/detail.gif",
          "assets/citytex/water/12_DIFFUSE.jpg",
          "assets/citytex/water/12_DISP.jpg",
          "assets/citytex/water/12_NORMAL.jpg",
          "assets/citytex/models/lamp/lamp_bn.gif",
          "assets/citytex/models/trafficLight/traffic.gif",
          "assets/citytex/models/hydrant/hydrant.gif",
          "assets/citytex/models/postbox/postbox.gif",
          "assets/citytex/models/tree/tree.gif",
          "assets/citytex/models/tree/tree2.gif",
          "assets/citytex/models/bench/bench.gif",
          "assets/citytex/models/bin/bin.gif",
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

    CityMesh() {

    }

    void setHeightmap(HeightMap *hm) {
      heightMap = hm;
    }

    // Samples the heights of the heightmap to create a projected road on the surface
    // result - the array of float with the heights
    // v1, v2 - start and end of the road
    // points - number of points to sample. Function will sample points+1
    // cityDimensions - the size of the city. Terrain will always be centered at the geometric center of the city
    // cityCenter - the geometric center of the city
    // multiplier - The terrain is grown a times the size of the city, this is the factor
    // offsetX, offsetY - The city is centered in the terrain, this allows to center the height samples
    void get_road_heights(dynarray<float> &result, vec4 &v1, vec4 &v2, int points, vec4 &cityDimensions, vec4 &cityCenter, float multiplier, float offsetX, float offsetY) {
      result.reset();

      image *heightmapImage = ((*getImageArray())[TEXTUREASSET_HEIGHTMAP]);

      vec4 vDiff = v2 - v1;

      for (int i = 0; i != points+1; i++) {
        float t = ((float)i)/((float)points);

        vec4 vt = v1 + vDiff*t;
        vec4 color;

        float u_ = (vt.x()+cityCenter.x()+cityDimensions.x()*0.5f) / (cityDimensions.x());
        float v_ = (vt.z()+cityCenter.z()+cityDimensions.z()*0.5f) / (cityDimensions.z());

        u_ = ((1.0f-u_)*multiplier)+offsetX;
        v_ = (v_*multiplier)+offsetY;

        heightmapImage->sample2Dbilinear(u_, v_, color);
        //printf("Point %.2f (%.2f, %.2f, %.2f, %.2f), Sampling hm at (%.2f, %.2f) = %.2f\n", t, vt.x(), vt.y(), vt.z(), vt.w(), u_, v_, color.x()/255.0f*2.0f);

        float h = (color.x() * CityConstants::HEIGHT_FACTOR)+CityConstants::HEIGHT_FACTOR*3;
        if (h < CityConstants::BRIDGE_LEVEL) {
          h = CityConstants::BRIDGE_LEVEL;
        }
        result.push_back(h);
      }
    }

    void init(dynarray<Street> *streetsList, dynarray<BuildingArea> *buildingAreaList, vec4 &cityDimensions, vec4 &cityCenter) {
    // temp
    dynarray<BuildingArea> buildingAreaList2; 

      mesh_builder mb;
      mesh_builder mbRoadLeft;
      mesh_builder mbRoadRight;
      mesh_builder mbPavement;

      //Create heightmap
      vec4 terrainDimensions = cityDimensions*2.0f;

      printf("Creating road meshes.\n");

      printf("Creating surface from heightmap.\n");
      mb.init(0, 0);
      mb.translate(cityCenter.x(), cityCenter.y(), cityCenter.z());
      mb.rotate(-90, 1, 0, 0);
      mb.add_plane_heightmap(terrainDimensions.x(), terrainDimensions.z(), heightMap->getWidth()-2, heightMap->getHeight()-2, heightMap->getNormalMapXY(), heightMap->getWidth(), heightMap->getHeight(), heightMap->getHeightmap(), 0.0f, 0.0f, 10, 13);
      mb.get_mesh(surfaceMesh);
      //surfaceMesh.set_mode(GL_LINE_STRIP);
  
      printf("Creating water plane.\n");
      mb.init(0, 0);
      mb.translate(cityCenter.x(), cityCenter.y()+CityConstants::WATER_LEVEL, cityCenter.z());
      mb.rotate(-90, 1, 0, 0);
      mb.add_plane(terrainDimensions.x(), terrainDimensions.z(), 10, 10);
      mb.get_mesh(waterMesh);

      float separationX = terrainDimensions.x()/(heightMap->getWidth()-2);
      float separationZ = terrainDimensions.z()/(heightMap->getHeight()-2);
      int gridWidth = heightMap->getWidth()-2;
      int gridHeight = heightMap->getHeight()-2;

      // Creating road meshes
      printf("Creating road meshes.\n");
      mbRoadLeft.init(0, 0);
      mbRoadRight.init(0, 0);
      mbPavement.init(0, 0);

      for (int i = 0; i < streetsList->size(); i++) {
        Street &street = (*streetsList)[i];
        dynarray<float> road_heights;

        // Creating road
        vec4 v1 = street.points[0];
        vec4 v2 = street.points[1];

        street.intersectGridStreet(cityCenter.x(), cityCenter.z(), separationX, separationZ, CityConstants::ROAD_HEIGHT, CityConstants::PAVEMENT_HEIGHT, gridWidth, gridHeight);

        for (auto j = street.terrainIntersectedPoints.roadLeft.begin(); j != street.terrainIntersectedPoints.roadLeft.end(); j++) {
          (*j)[1] += heightMap->sample_heightmap(*j) + CityConstants::ROAD_RAISE;
        }

        for (auto j = street.terrainIntersectedPoints.roadRight.begin(); j != street.terrainIntersectedPoints.roadRight.end(); j++) {
          (*j)[1] += heightMap->sample_heightmap(*j) + CityConstants::ROAD_RAISE;
        }

        for (auto j = street.terrainIntersectedPoints.pavementLeft.begin(); j != street.terrainIntersectedPoints.pavementLeft.end(); j++) {
          (*j)[1] += heightMap->sample_heightmap(*j) + CityConstants::PAVEMENT_RAISE;
        }

        for (auto j = street.terrainIntersectedPoints.pavementRight.begin(); j != street.terrainIntersectedPoints.pavementRight.end(); j++) {
          (*j)[1] += heightMap->sample_heightmap(*j) + CityConstants::PAVEMENT_RAISE;
        }
        
        if (street.terrainIntersectedPoints.roadLeft.size() > 0) {
          mbRoadRight.add_vertices(street.terrainIntersectedPoints.roadLeft,
                                   street.terrainIntersectedIndices.roadLeft,
                                   street.terrainIntersectedNormals.roadLeft,
                                   street.terrainIntersectedUVCoords.roadLeft,
                                   cityDimensions, cityCenter, CityConstants::MULTIPLIER, CityConstants::OFFSET_X, CityConstants::OFFSET_Y,
                                   heightMap->getWidth()-2, heightMap->getHeight()-2, heightMap->getNormalMapXZ(), heightMap->getWidth(), heightMap->getHeight(), heightMap->getHeightmap());
        }
        
        if (street.terrainIntersectedPoints.roadRight.size() > 0) {
          mbRoadLeft.add_vertices(street.terrainIntersectedPoints.roadRight,
                                  street.terrainIntersectedIndices.roadRight,
                                  street.terrainIntersectedNormals.roadRight,
                                  street.terrainIntersectedUVCoords.roadRight,
                                  cityDimensions, cityCenter, CityConstants::MULTIPLIER, CityConstants::OFFSET_X, CityConstants::OFFSET_Y,
                                  heightMap->getWidth()-2, heightMap->getHeight()-2, heightMap->getNormalMapXZ(), heightMap->getWidth(), heightMap->getHeight(), heightMap->getHeightmap());
        }

        // Right Pavement
        if (street.terrainIntersectedPoints.pavementRight.size() > 0) {
          mbPavement.add_vertices(street.terrainIntersectedPoints.pavementRight,
                                  street.terrainIntersectedIndices.pavementRight,
                                  street.terrainIntersectedNormals.pavementRight,
                                  street.terrainIntersectedUVCoords.pavementRight,
                                  cityDimensions, cityCenter, CityConstants::MULTIPLIER, CityConstants::OFFSET_X, CityConstants::OFFSET_Y,
                                  heightMap->getWidth()-2, heightMap->getHeight()-2, heightMap->getNormalMapXZ(), heightMap->getWidth(), heightMap->getHeight(), heightMap->getHeightmap());
        }
        
        // Left Pavement
        if (street.terrainIntersectedPoints.pavementLeft.size() > 0) {
          mbPavement.add_vertices(street.terrainIntersectedPoints.pavementLeft,
                                  street.terrainIntersectedIndices.pavementLeft,
                                  street.terrainIntersectedNormals.pavementLeft,
                                  street.terrainIntersectedUVCoords.pavementLeft,
                                  cityDimensions, cityCenter, CityConstants::MULTIPLIER, CityConstants::OFFSET_X, CityConstants::OFFSET_Y,
                                  heightMap->getWidth()-2, heightMap->getHeight()-2, heightMap->getNormalMapXZ(), heightMap->getWidth(), heightMap->getHeight(), heightMap->getHeightmap());
        }
      }

      mbRoadLeft.get_mesh(roadLeftMesh); 
      mbRoadRight.get_mesh(roadRightMesh); 
      mbPavement.get_mesh(pavementMesh);

      roadLeftNormalsMesh.make_normal_visualizer(roadLeftMesh, 0.3f, attribute_normal);
      roadRightNormalsMesh.make_normal_visualizer(roadRightMesh, 0.3f, attribute_normal);
      pavementNormalsMesh.make_normal_visualizer(pavementMesh, 0.3f, attribute_normal);
      surfaceNormalsMesh.make_normal_visualizer(surfaceMesh, 0.3f, attribute_normal);

    
  


    // creating buildings meshes 
    printf("Creating buildings.\n");
      for (int i = 0; i < buildingAreaList->size(); i++) {
        mb.init(0, 0);
        
        float random_height = std::rand()%4 + 2;

    // central mesh of the building
        mb.add_extrude_polygon((*buildingAreaList)[i].points, random_height, CityConstants::BUILDING_BASEMENT_HEIGHT); 
    (*buildingAreaList)[i].height = random_height; 
    (*buildingAreaList)[i].calculate_area();
        
        mesh * m = new mesh();
        mb.get_mesh(*m);
        m->set_mode(GL_TRIANGLES);
        (*buildingAreaList)[i].areaMesh = (*m);

    // basement mesh of the building
    mb.init(0,0); 
    mb.add_basement((*buildingAreaList)[i].points, CityConstants::BUILDING_BASEMENT_HEIGHT);
    m->init();
    mb.get_mesh(*m);
    m->set_mode(GL_TRIANGLES);
    (*buildingAreaList)[i].basementMesh = (*m);

    // roof mesh of the building
    mb.init(0,0); 
    mb.add_roof((*buildingAreaList)[i].points, CityConstants::BUILDING_BASEMENT_HEIGHT + random_height);
    m->init();
    mb.get_mesh(*m);
    m->set_mode(GL_TRIANGLES);
    (*buildingAreaList)[i].roofMesh = (*m);
      }

      pavementMaterial = new material((*getImageArray())[TEXTUREASSET_PAVEMENT]);
      roadMaterialLeft = new material((*getImageArray())[TEXTUREASSET_ROADLEFT]);
      roadMaterialRight = new material((*getImageArray())[TEXTUREASSET_ROADRIGHT]);
      grassMaterial = new material((*getImageArray())[TEXTUREASSET_GRASS_DIFFUSE], (*getImageArray())[TEXTUREASSET_GRASS_NORMAL]);
      buldingMaterial = new material((*getImageArray())[TEXTUREASSET_BUILDING], (*getImageArray())[TEXTUREASSET_BUILDING_RES_1], (*getImageArray())[TEXTUREASSET_BUILDING_RES_2], (*getImageArray())[TEXTUREASSET_BUILDING_RES_3]);
      (*getImageArray())[TEXTUREASSET_WATER_DIFFUSE]->multiplyColor(vec4(1.0f, 1.0f, 1.0f, 0.5f));
      waterMaterial = new material((*getImageArray())[TEXTUREASSET_WATER_DIFFUSE], (*getImageArray())[TEXTUREASSET_WATER_NORMAL]);

      lampMaterial = new material((*getImageArray())[TEXTUREASSET_LAMP_TEXTURE]);
      trafficLightMaterial = new material((*getImageArray())[TEXTUREASSET_TRAFFICLIGHT_TEXTURE]);
      hydrantMaterial = new material((*getImageArray())[TEXTUREASSET_HYDRANT_TEXTURE]);
      postBoxMaterial = new material((*getImageArray())[TEXTUREASSET_POSTBOX_TEXTURE]);
      treeMaterial = new material((*getImageArray())[TEXTUREASSET_TREE_TEXTURE]);
      tree2Material = new material((*getImageArray())[TEXTUREASSET_TREE2_TEXTURE]);
      benchMaterial = new material((*getImageArray())[TEXTUREASSET_BENCH_TEXTURE]);
      binMaterial = new material((*getImageArray())[TEXTUREASSET_BIN_TEXTURE]);

      skyboxMesh.make_cube(100.0f);
      sky_box_textureObj = 0;
      createCubeMap();
    }


    void createCubeMap(){

      glGenTextures(1, &sky_box_textureObj);
      glBindTexture(GL_TEXTURE_CUBE_MAP, sky_box_textureObj);

      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, 2048, 2048, 0, GL_RGBA, GL_UNSIGNED_BYTE, getImage("assets/citytex/skybox11.gif"));
      glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, 2048, 2048, 0, GL_RGBA, GL_UNSIGNED_BYTE, getImage("assets/citytex/skybox22.gif"));
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, 2048, 2048, 0, GL_RGBA, GL_UNSIGNED_BYTE, getImage("assets/citytex/skybox33.gif"));
      glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, 2048, 2048, 0, GL_RGBA, GL_UNSIGNED_BYTE, getImage("assets/citytex/skybox44.gif"));
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, 2048, 2048, 0, GL_RGBA, GL_UNSIGNED_BYTE, getImage("assets/citytex/skybox55.gif"));
      glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, 2048, 2048, 0, GL_RGBA, GL_UNSIGNED_BYTE, getImage("assets/citytex/skybox66.gif"));

      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    uint8_t* getImage(const char *url){

      buffer.reset();
      images.reset();

      app_utils::get_url(buffer,url);
      uint16_t format = 0;
      uint16_t width = 0;
      uint16_t height = 0;
      const unsigned char *src = &buffer[0];
      const unsigned char *src_max = src + buffer.size();

      gif_decoder dec;
      dec.get_image(images, format, width, height, src, src_max);

      return &images[0];
    }

    void debugRender(bump_shader &shader, city_buildings_bump_shader &buldingShader, color_shader &cshader, skybox_shader &sb_shader,const mat4t &modelToProjection, const mat4t &modelToCamera, const mat4t &cameraToWorld, vec4 *light_uniforms, const int num_light_uniforms, const int num_lights,
        dynarray<BuildingArea> *buildingAreaList,std::vector <ref<Model>> *models,int drawFlags, int draw_texture_mode) {

      if (drawFlags & 0x1) {
        grassMaterial->render(shader, modelToProjection, modelToCamera, light_uniforms, num_light_uniforms, num_lights);
        surfaceMesh.render();
      }

      if (drawFlags & 0x4) {
        roadMaterialLeft->render(shader, modelToProjection, modelToCamera, light_uniforms, num_light_uniforms, num_lights);
        roadLeftMesh.render();

        roadMaterialRight->render(shader, modelToProjection, modelToCamera, light_uniforms, num_light_uniforms, num_lights);
        roadRightMesh.render();

        pavementMaterial->render(shader, modelToProjection, modelToCamera, light_uniforms, num_light_uniforms, num_lights);
        pavementMesh.render();
      }

      if (drawFlags & 0x8) {
       
      for (int i = 0; i != buildingAreaList->size(); ++i) {
      buldingMaterial->render_building(buldingShader, modelToProjection, modelToCamera, light_uniforms, num_light_uniforms, num_lights, (*buildingAreaList)[i].height, (*buildingAreaList)[i].area, draw_texture_mode, 0);
      (*buildingAreaList)[i].areaMesh.render();
      buldingMaterial->render_building(buldingShader, modelToProjection, modelToCamera, light_uniforms, num_light_uniforms, num_lights, (*buildingAreaList)[i].height, (*buildingAreaList)[i].area, draw_texture_mode, 1);
      (*buildingAreaList)[i].roofMesh.render();
      buldingMaterial->render_building(buldingShader, modelToProjection, modelToCamera, light_uniforms, num_light_uniforms, num_lights, (*buildingAreaList)[i].height, (*buildingAreaList)[i].area, draw_texture_mode, 2);
      (*buildingAreaList)[i].basementMesh.render();
        }
      }

      if (drawFlags & 0x2) {
        waterMaterial->render(shader, modelToProjection, modelToCamera, light_uniforms, num_light_uniforms, num_lights);
        waterMesh.render();
      }

      if (drawFlags & 0x40) {
        cshader.render(modelToProjection, vec4(0.0f, 0.0f, 1.0f, 1.0f));
        surfaceNormalsMesh.render();
      }

      if (drawFlags & 0x80) {
        cshader.render(modelToProjection, vec4(0.0f, 0.0f, 1.0f, 1.0f));
        roadLeftNormalsMesh.render();
        roadRightNormalsMesh.render();
        pavementNormalsMesh.render();
      }

      //RENDER 3D MODELS

      for(int i=0;i!=models->size();++i){
        if((*models)[i]->getMaterial() == "Lamp"){
          lampMaterial->render(shader, (*models)[i]->getModelToWorld()*modelToProjection, (*models)[i]->getModelToWorld()*modelToCamera, light_uniforms, num_light_uniforms, num_lights);
        }

        if((*models)[i]->getMaterial() == "Traffic Light"){
          trafficLightMaterial->render(shader, (*models)[i]->getModelToWorld()*modelToProjection, (*models)[i]->getModelToWorld()*modelToCamera, light_uniforms, num_light_uniforms, num_lights);
        }

        if((*models)[i]->getMaterial() == "Hydrant"){
          hydrantMaterial->render(shader, (*models)[i]->getModelToWorld()*modelToProjection, (*models)[i]->getModelToWorld()*modelToCamera, light_uniforms, num_light_uniforms, num_lights);
        }

        if((*models)[i]->getMaterial() == "Postbox"){
          postBoxMaterial->render(shader, (*models)[i]->getModelToWorld()*modelToProjection, (*models)[i]->getModelToWorld()*modelToCamera, light_uniforms, num_light_uniforms, num_lights);
        }

        if((*models)[i]->getMaterial() == "Tree"){
          treeMaterial->render(shader, (*models)[i]->getModelToWorld()*modelToProjection, (*models)[i]->getModelToWorld()*modelToCamera, light_uniforms, num_light_uniforms, num_lights);
        }

        if((*models)[i]->getMaterial() == "Tree2"){
          tree2Material->render(shader, (*models)[i]->getModelToWorld()*modelToProjection, (*models)[i]->getModelToWorld()*modelToCamera, light_uniforms, num_light_uniforms, num_lights);
        }

        if((*models)[i]->getMaterial() == "Bench"){
          benchMaterial->render(shader, (*models)[i]->getModelToWorld()*modelToProjection, (*models)[i]->getModelToWorld()*modelToCamera, light_uniforms, num_light_uniforms, num_lights);
        }

        if((*models)[i]->getMaterial() == "Bin"){
          binMaterial->render(shader, (*models)[i]->getModelToWorld()*modelToProjection, (*models)[i]->getModelToWorld()*modelToCamera, light_uniforms, num_light_uniforms, num_lights);
        }

        (*models)[i]->render();
      }

      glActiveTexture(GL_TEXTURE7);
      glBindTexture(GL_TEXTURE_CUBE_MAP,sky_box_textureObj);

      mat4t skyToWorld = mat4t(1.0f);
      skyToWorld.translate(cameraToWorld.row(3).x(), cameraToWorld.row(3).y(), cameraToWorld.row(3).z());

      mat4t mTP = modelToProjection;

      mTP = mat4t::build_projection_matrix(skyToWorld, cameraToWorld);

      sb_shader.render(mTP,modelToCamera,GL_TEXTURE7);

      skyboxMesh.render();
    }

    void set_mode(unsigned int drawFlags, dynarray<BuildingArea> *buildingAreaList) {
      unsigned int WIREFRAME_MODE = GL_LINE_LOOP; 
      if (drawFlags & 0x100) {
        surfaceMesh.set_mode(WIREFRAME_MODE);
      } else {
        surfaceMesh.set_mode(GL_TRIANGLES);
      }

      if (drawFlags & 0x200) {
        roadLeftMesh.set_mode(WIREFRAME_MODE);
        roadRightMesh.set_mode(WIREFRAME_MODE);
        pavementMesh.set_mode(WIREFRAME_MODE);
      } else {
        roadLeftMesh.set_mode(GL_TRIANGLES);
        roadRightMesh.set_mode(GL_TRIANGLES);
        pavementMesh.set_mode(GL_TRIANGLES);
      }

      if (drawFlags & 0x400) {
        for (int i = 0; i != buildingAreaList->size(); ++i) {
          (*buildingAreaList)[i].areaMesh.set_mode(WIREFRAME_MODE);
        }
      } else {
        for (int i = 0; i != buildingAreaList->size(); ++i) {
          (*buildingAreaList)[i].areaMesh.set_mode(GL_TRIANGLES);
        }
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

    void render(vec4 *camera_position, vec3 *camera_rotation, float aspectRatio, vec4 &light_direction) {
      mat4t modelToWorld;
      modelToWorld.loadIdentity();

      mat4t cameraToWorld;
      cameraToWorld.loadIdentity();
      cameraToWorld.rotate((*camera_rotation)[1], 0.0f, 1.0f, 0.0f);
      cameraToWorld.rotate(-(*camera_rotation)[0], 1.0f, 0.0f, 0.0f);
      cameraToWorld.translate(0.0f, 0.0f, 10.0f); //camera_position->z());

      mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld, 0.1f, 1000.0f, 0.08f, 0.08f*aspectRatio, 0.1f*aspectRatio);

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
      
      float light_arrow[] = {
        0.0f, 0.0f, 0.0f,
        light_direction.x(), light_direction.y(), light_direction.z()
      };
      
      float light_arrow_proj[] = {
        light_direction.x(), 0.0f, 0.0f,
        light_direction.x(), 0.0f, light_direction.z(),
        light_direction.x(), 0.0f, light_direction.z(),
        0.0f, 0.0f, light_direction.z(),
        light_direction.x(), 0.0f, light_direction.z(),
        light_direction.x(), light_direction.y(), light_direction.z(),
        light_direction.x(), 0.0f, 0.0f,
        light_direction.x(), light_direction.y(), 0.0f,
        light_direction.x(), light_direction.y(), 0.0f,
        0.0f, light_direction.y(), 0.0f,
        light_direction.x(), light_direction.y(), 0.0f,
        light_direction.x(), light_direction.y(), light_direction.z(),
        0.0f, 0.0f, light_direction.z(),
        0.0f, light_direction.y(), light_direction.z(),
        0.0f, light_direction.y(), light_direction.z(),
        0.0f, light_direction.y(), 0.0f,
        0.0f, light_direction.y(), light_direction.z(),
        light_direction.x(), light_direction.y(), light_direction.z()
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
      
      cshader->render(modelToProjection, vec4(1.0f, 1.0f, 1.0f, 1.0f));
      glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 0, light_arrow);
      glDrawArrays(GL_LINES, 0, 2);

      cshader->render(modelToProjection, vec4(0.4f, 0.4f, 0.4f, 0.6f));
      glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 0, light_arrow_proj);
      glDrawArrays(GL_LINES, 0, 18);
      
      glDisableVertexAttribArray(attribute_pos);
      glEnable(GL_DEPTH_TEST);
    }
  };


}