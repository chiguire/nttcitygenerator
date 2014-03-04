namespace octet {

  const float HEIGHT_FACTOR = 2.0f/255.0f;
  const float WATER_LEVEL = 0.5f;
  const float BRIDGE_LEVEL = 0.8f;

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

    dynarray<float> heightmap;
    dynarray<vec4> normalmapXY;
    dynarray<vec4> normalmapXZ;
    int heightmap_width;
    int heightmap_height;

    static dynarray<image *> *imageArray_;

    mesh skyboxMesh;
    GLuint sky_box_textureObj;
    dynarray<uint8_t> buffer;
    dynarray<uint8_t> images;

    static dynarray<image *> *getImageArray() {
      if (!imageArray_) {
        imageArray_ = new dynarray<image *>();
        char *files[] = {
          "assets/citytex/pavement.gif",
          "assets/citytex/road_left.gif",
          "assets/citytex/road_right.gif",
          "assets/citytex/grass_3.gif",
          "assets/citytex/heightmap6.gif",
          "assets/citytex/water_2.gif",
		  "assets/citytex/building_h.gif",
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

    }

    void generateHeightmap() {
      heightmap.reset();
      image *heightmapImage = ((*getImageArray())[4]);
      heightmap_width = heightmapImage->get_width()+2;
      heightmap_height = heightmapImage->get_height()+2;
      heightmap.resize(heightmap_width*heightmap_height);

      for (int j = 0; j != heightmap_height; j++) {
        float v_ = ((float)j-1) / (heightmap_height-2);
        for (int i = 0; i != heightmap_width; i++) {
          vec4 color;

          float u_ = ((float)i-1) / (heightmap_width-2);

          heightmapImage->sample2Dbilinear(u_, v_, color);

          heightmap[j*heightmap_width+i] = ((color.x()+color.y()+color.z())/3.0f)*HEIGHT_FACTOR;

        }
      }
    }

    void generateNormalMap() {
      image *heightmapImage = ((*getImageArray())[4]);
      unsigned int nx = heightmapImage->get_width();
      unsigned int ny = heightmapImage->get_height();
      unsigned int hmx = nx + 2;
      unsigned int hmy = ny + 2;

      normalmapXY.reset();
      normalmapXY.resize(nx*ny);      
      normalmapXZ.reset();
      normalmapXZ.resize(nx*ny);      

      mat4t rotMat;
      rotMat.loadIdentity();
      rotMat.rotate(-90, 1.0f, 0.0f, 0.0f);

      for (unsigned j = 0; j != ny; j++) {
        for (unsigned i = 0; i != nx; i++) {
          vec4 normal(0.0f, 0.0f, 0.0f, 1.0f);

          float h01 = heightmap[hmx*(j+1)+(i+1-1)];
          float h21 = heightmap[hmx*(j+1)+(i+1+1)];
          float h10 = heightmap[hmx*(j+1-1)+(i+1)];
          float h12 = heightmap[hmx*(j+1+1)+(i+1)];
          vec3 va(2.0f, 0.0f, h21 - h01);
          vec3 vb(0.0f, 2.0f, h12 - h10);
          vec3 c = va.cross(vb);
          normal = vec4(c[0], c[1], c[2], 1.0f);
          normal = normal.normalize();
          normalmapXY[nx*j+i] = normal;
          normalmapXZ[nx*j+i] = normal*rotMat;
        }
      }
    }

    /*
    //Not yet functional, didn't manage to get it working in time
    //Calculate the bezier point
    vec4 drawBezierGeneralized(vec4 &firstPoint, vec4 &secondPoint, vec4 &thirdPoint, vec4 &fourthPoint) {
    vec4 pointOfResult = vec4(0,0,0,0);
    float points = 4;

    for(int i=0;i<4;i++)
    {
    if(i == 0) {
    pointOfResult.x = firstPoint.x + binomial_coff(points,i) * pow(points,i) * pow((1-points),(points-i)) * firstPoint.x;
    pointOfResult.y = firstPoint.y + binomial_coff(points,i) * pow(points,i) * pow((1-points),(points-i)) * firstPoint.y;
    }
    if(i == 1) {
    pointOfResult.x = secondPoint.x + binomial_coff(points,i) * pow(points,i) * pow((1-points),(points-i)) * secondPoint.x;
    pointOfResult.y = firstPoint.y + binomial_coff(points,i) * pow(points,i) * pow((1-points),(points-i)) * secondPoint.y;
    }
    if(i == 2) {
    pointOfResult.x = thirdPoint.x + binomial_coff(points,i) * pow(points,i) * pow((1-points),(points-i)) * thirdPoint.x;
    pointOfResult.y = firstPoint.y + binomial_coff(points,i) * pow(points,i) * pow((1-points),(points-i)) * thirdPoint.y;
    }
    if(i == 3) {
    pointOfResult.x = fourthPoint.x + binomial_coff(points,i) * pow(points,i) * pow((1-points),(points-i)) * fourthPoint.x;
    pointOfResult.y = firstPoint.y + binomial_coff(points,i) * pow(points,i) * pow((1-points),(points-i)) * fourthPoint.y;
    }
    }
    //cout<<P.x<<endl<<P.y;
    //cout<<endl<<endl;
    return pointOfResult;
    }


    int factorial(int n)
    {
    if (n<=1)
    return(1);
    else
    n=n*factorial(n-1);
    return n;
    }

    float binomial_coff(float n,float k)
    {
    float ans;
    ans = factorial(n) / (factorial(k)*factorial(n-k));
    return ans;
    }
    */



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

      image *heightmapImage = ((*getImageArray())[4]);

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

        float h = (color.x() * HEIGHT_FACTOR)+HEIGHT_FACTOR*3;
        if (h < BRIDGE_LEVEL) {
          h = BRIDGE_LEVEL;
        }
        result.push_back(h);
      }
    }

    float sample_heightmap(vec4 &vertex, vec4 &cityDimensions, vec4 &cityCenter, float multiplier, float offsetX, float offsetY) {
      image *heightmapImage = ((*getImageArray())[4]);
      vec4 color;

      float u_ = (vertex.x()-cityCenter.x()+cityDimensions.x()*0.5f) / (cityDimensions.x());
      float v_ = (vertex.z()-cityCenter.z()+cityDimensions.z()*0.5f) / (cityDimensions.z());

      u_ = (u_*multiplier)+offsetX;
      v_ = ((1-v_)*multiplier)+offsetY;

      heightmapImage->sample2Dbilinear(u_, v_, color);

      float h = (color.x() * HEIGHT_FACTOR);
      if (h < BRIDGE_LEVEL) {
        h = BRIDGE_LEVEL;
      }
      return h;
    }


    void init(dynarray<Street> *streetsList, dynarray<BuildingArea> *buildingAreaList, vec4 &cityDimensions, vec4 &cityCenter) {
      mesh_builder mb;
      mesh_builder mbRoadLeft;
      mesh_builder mbRoadRight;
      mesh_builder mbPavement;

      //Create heightmap
      vec4 terrainDimensions = cityDimensions*2.0f;

      printf("Generating heightmap and normalmap.\n");
      generateHeightmap();
      generateNormalMap();

      printf("Creating road meshes.\n");

      printf("Creating surface from heightmap.\n");
      mb.init(0, 0);
      mb.translate(cityCenter.x(), cityCenter.y(), cityCenter.z());
      mb.rotate(-90, 1, 0, 0);
      mb.add_plane_heightmap(terrainDimensions.x(), terrainDimensions.z(), heightmap_width-2, heightmap_height-2, normalmapXY.data(), heightmap_width, heightmap_height, heightmap.data());
      mb.get_mesh(surfaceMesh);
      //surfaceMesh.set_mode(GL_LINE_STRIP);
  
      printf("Creating water plane.\n");
      mb.init(0, 0);
      mb.translate(cityCenter.x(), cityCenter.y()+WATER_LEVEL, cityCenter.z());
      mb.rotate(-90, 1, 0, 0);
      mb.add_plane(terrainDimensions.x(), terrainDimensions.z(), 10, 10);
      mb.get_mesh(waterMesh);

      float separationX = terrainDimensions.x()/(heightmap_width-2);
      float separationZ = terrainDimensions.z()/(heightmap_height-2);
      int gridWidth = heightmap_width-2;
      int gridHeight = heightmap_height-2;

      // Creating road meshes
      mbRoadLeft.init(0, 0);
      mbRoadRight.init(0, 0);
      mbPavement.init(0, 0);

	  // creating buildings
      for (int i = 0; i < buildingAreaList->size(); i++) {
        mb.init(0, 0);
        
		float random_height = std::rand()%5 +1;
        mb.add_extrude_polygon((*buildingAreaList)[i].points, random_height); 
        
        mesh * m = new mesh();
        mb.get_mesh(*m);
        m->set_mode(GL_TRIANGLES);
        (*buildingAreaList)[i].areaMesh = (*m);
      }

      const float MULTIPLIER = 0.5f;
      const float OFFSET_X = 0.25f;
      const float OFFSET_Y = 0.25f;
      const float ROAD_RAISE = City::ROAD_HEIGHT;
      const float PAVEMENT_RAISE = City::PAVEMENT_HEIGHT;

      for (int i = 0; i < streetsList->size(); i++) {
        Street &street = (*streetsList)[i];
        dynarray<float> road_heights;

        // Creating road
        vec4 v1 = street.points[0];
        vec4 v2 = street.points[1];

        street.intersectGridStreet(cityCenter.x(), cityCenter.z(), separationX, separationZ, City::ROAD_HEIGHT, City::PAVEMENT_HEIGHT, gridWidth, gridHeight);

        for (auto j = street.roadMeshLeftIntersectedPoints.begin(); j != street.roadMeshLeftIntersectedPoints.end(); j++) {
          (*j)[1] += sample_heightmap(*j, cityDimensions, cityCenter, MULTIPLIER, OFFSET_X, OFFSET_Y) + ROAD_RAISE;
        }

        for (auto j = street.roadMeshRightIntersectedPoints.begin(); j != street.roadMeshRightIntersectedPoints.end(); j++) {
          (*j)[1] += sample_heightmap(*j, cityDimensions, cityCenter, MULTIPLIER, OFFSET_X, OFFSET_Y) + ROAD_RAISE;
        }

        for (auto j = street.pavementMeshLeftIntersectedPoints.begin(); j != street.pavementMeshLeftIntersectedPoints.end(); j++) {
          (*j)[1] += sample_heightmap(*j, cityDimensions, cityCenter, MULTIPLIER, OFFSET_X, OFFSET_Y) + PAVEMENT_RAISE;
        }

        for (auto j = street.pavementMeshRightIntersectedPoints.begin(); j != street.pavementMeshRightIntersectedPoints.end(); j++) {
          (*j)[1] += sample_heightmap(*j, cityDimensions, cityCenter, MULTIPLIER, OFFSET_X, OFFSET_Y) + PAVEMENT_RAISE;
        }
        
        if (street.roadMeshLeftIntersectedPoints.size() > 0) {
          mbRoadRight.add_vertices(street.roadMeshLeftIntersectedPoints, street.roadMeshLeftIntersectedIndices, street.roadMeshLeftIntersectedNormals, street.roadMeshLeftIntersectedUVCoords,
                                   cityDimensions, cityCenter, MULTIPLIER, OFFSET_X, OFFSET_Y,
                                   heightmap_width-2, heightmap_height-2, normalmapXZ.data(), heightmap_width, heightmap_height, heightmap.data());
        }
        
        if (street.roadMeshRightIntersectedPoints.size() > 0) {
          mbRoadLeft.add_vertices(street.roadMeshRightIntersectedPoints, street.roadMeshRightIntersectedIndices, street.roadMeshRightIntersectedNormals, street.roadMeshRightIntersectedUVCoords,
                                  cityDimensions, cityCenter, MULTIPLIER, OFFSET_X, OFFSET_Y,
                                  heightmap_width-2, heightmap_height-2, normalmapXZ.data(), heightmap_width, heightmap_height, heightmap.data());
        }

        // Right Pavement
        if (street.pavementMeshRightIntersectedPoints.size() > 0) {
          mbPavement.add_vertices(street.pavementMeshRightIntersectedPoints, street.pavementMeshRightIntersectedIndices, street.pavementMeshRightIntersectedNormals, street.pavementMeshLeftIntersectedUVCoords,
                                  cityDimensions, cityCenter, MULTIPLIER, OFFSET_X, OFFSET_Y,
                                  heightmap_width-2, heightmap_height-2, normalmapXZ.data(), heightmap_width, heightmap_height, heightmap.data());
        }
        
        // Left Pavement
        if (street.pavementMeshLeftIntersectedPoints.size() > 0) {
          mbPavement.add_vertices(street.pavementMeshLeftIntersectedPoints, street.pavementMeshLeftIntersectedIndices, street.pavementMeshLeftIntersectedNormals, street.pavementMeshRightIntersectedUVCoords,
                                  cityDimensions, cityCenter, MULTIPLIER, OFFSET_X, OFFSET_Y,
                                  heightmap_width-2, heightmap_height-2, normalmapXZ.data(), heightmap_width, heightmap_height, heightmap.data());
        }
      }

      mbRoadLeft.get_mesh(roadLeftMesh); 
      mbRoadRight.get_mesh(roadRightMesh); 
      mbPavement.get_mesh(pavementMesh);

      roadLeftNormalsMesh.make_normal_visualizer(roadLeftMesh, 0.3f, attribute_normal);
      roadRightNormalsMesh.make_normal_visualizer(roadRightMesh, 0.3f, attribute_normal);
      pavementNormalsMesh.make_normal_visualizer(pavementMesh, 0.3f, attribute_normal);
      surfaceNormalsMesh.make_normal_visualizer(surfaceMesh, 0.3f, attribute_normal);

      pavementMaterial = new material((*getImageArray())[0]);
      roadMaterialLeft = new material((*getImageArray())[1]);
      roadMaterialRight = new material((*getImageArray())[2]);
      grassMaterial = new material((*getImageArray())[3], false);
	    buldingMaterial = new material((*getImageArray())[6]);
      waterMaterial = new material((*getImageArray())[5], vec4(0.1f, 0.2f, 0.8f, 0.5f), true, true);


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
        dynarray<BuildingArea> *buildingAreaList, int drawFlags) {

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
        buldingMaterial->renderBuilding(buldingShader, modelToProjection, modelToCamera, light_uniforms, num_light_uniforms, num_lights);
        for (int i = 0; i != buildingAreaList->size(); ++i) {
          (*buildingAreaList)[i].areaMesh.render();
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