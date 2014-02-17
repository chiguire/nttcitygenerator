namespace octet {

  const float HEIGHT_FACTOR = 2.0f/255.0f;
  const float WATER_LEVEL = 0.5f;
  const float BRIDGE_LEVEL = 0.8f;

  class CityMesh {

    mesh surfaceMesh;
    mesh waterMesh;

    material *roadMaterialLeft;
    material *roadMaterialRight;
    material *pavementMaterial;
    material *grassMaterial;
    material *waterMaterial;

    dynarray<float> heightmap;
    int heightmap_width;
    int heightmap_height;
    
    static dynarray<image *> *imageArray_;

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

          heightmap[j*heightmap_height+i] = ((color.x()+color.y()+color.z())/3.0f)*HEIGHT_FACTOR;

          //printf("%.1f,", heightmap[j*heightmap_height+i]);
        }
        //printf("\n");
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


    void init(dynarray<Street> *streetsList, vec4 &cityDimensions, vec4 &cityCenter) {

      printf("Generating heightmap.\n");
      generateHeightmap(); 

      printf("Creating road meshes.\n");

      mesh_builder mb; 
      for (int i = 0; i < streetsList->size(); i++) {
        dynarray<float> road_heights;

        // Creating road
        mb.init(0, 0);
        vec4 v1 = (*streetsList)[i].points[0];
        vec4 v2 = (*streetsList)[i].points[1];

        //printf("Adding street %d, from (%.2f, %.2f) to (%.2f, %.2f)\n", (i+1), v1.x(), v1.z(), v2.x(), v2.z());

        float angleY = atan2f(v2.x() - v1.x(), v2.z() - v1.z())*180.0f/3.14159265359f;
        vec4 vMidpoint = vec4(v1.x() + (v2.x()-v1.x())/2.0f, v1.y() + (v2.y()-v1.y())/2.0f, v1.z() + (v2.z()-v1.z())/2.0f, 1.0f);

        float points_distance = (v2 - v1).length();
        unsigned int points = ceilf(points_distance*2);
        get_road_heights(road_heights, v1, v2, points, cityDimensions, cityCenter, 0.5f, 0.25f, 0.25f);

        mb.init(0, 0);
        
        ////bprintf("Midpoint (%.2f, %.2f, %.2f)\n", vMidpoint.x(), vMidpoint.y(), vMidpoint.z());
        //mb.translate(vMidpoint.x(), vMidpoint.y(), vMidpoint.z());
        //mb.rotate(angleY, 0.0f, 1.0f, 0.0f);
        //mb.add_cuboid(0.1f, 0.02f, points_distance/2.0f);
        ////mb.add_cuboid_heights(0.1f, 0.02f, points_distance/2.0f, points, road_heights.data());
        
        if((*streetsList)[i].roadMeshRightPoints.size() > 0)
        mb.add_cuboid_vertices(&((*streetsList)[i].roadMeshRightPoints));

        mesh * m = new mesh();
        mb.get_mesh(*m); 
        m->set_mode(GL_TRIANGLES);
        (*streetsList)[i].roadMeshRight = (*m);
       
        mb.init();

        if((*streetsList)[i].roadMeshLeftPoints.size() > 0)
        mb.add_cuboid_vertices(&((*streetsList)[i].roadMeshLeftPoints));

        m = new mesh();
        mb.get_mesh(*m); 
        m->set_mode(GL_TRIANGLES);
        (*streetsList)[i].roadMeshLeft = (*m);


        


        //Creating pavements
        //right

        mb.init(0, 0);

        if((*streetsList)[i].pavementMeshRightPoints.size() > 0 ){

          mb.add_cuboid_vertices(&((*streetsList)[i].pavementMeshRightPoints));


        }else{

          mb.translate(vMidpoint.x(), vMidpoint.y(), vMidpoint.z());
          mb.rotate(angleY, 0.0f, 1.0f, 0.0f);
          mb.translate(-0.1f-0.01f, 0.0f, 0.0f);
          mb.add_cuboid(0.02f, 0.04f, points_distance/2.0f);
          //mb.add_cuboid_heights(0.02f, 0.04f, points_distance/2.0f, points, road_heights.data());

        }

        
        
        m = new mesh();
        mb.get_mesh(*m); 
        m->set_mode(GL_TRIANGLES);
        (*streetsList)[i].pavementMeshRight = (*m);

        //Left

        mb.init(0, 0);

        if((*streetsList)[i].pavementMeshLeftPoints.size() > 0 ){
          mb.add_cuboid_vertices(&((*streetsList)[i].pavementMeshLeftPoints));
        }else{
  
          mb.translate(vMidpoint.x(), vMidpoint.y(), vMidpoint.z());
          mb.rotate(angleY, 0.0f, 1.0f, 0.0f);
          mb.translate(0.1f+0.01f, 0.0f, 0.0f);

          mb.add_cuboid(0.02f, 0.04f, points_distance/2.0f);
          //mb.add_cuboid_heights(0.02f, 0.04f, points_distance/2.0f, points, road_heights.data());
        }

        m = new mesh();
        mb.get_mesh(*m);
        m->set_mode(GL_TRIANGLES);
        (*streetsList)[i].pavementMeshLeft = (*m);
      }


      pavementMaterial = new material((*getImageArray())[0]);
      roadMaterialLeft = new material((*getImageArray())[1]);
      roadMaterialRight = new material((*getImageArray())[2]);
      grassMaterial = new material((*getImageArray())[3], false);
      // waterMaterial = new material((*getImageArray())[4]);
      waterMaterial = new material();
      waterMaterial->make_color(vec4(0.1f, 0.2f, 0.8f, 0.5f), true, true);
      //Create surface mesh

      
      //Create heightmap
      vec4 terrainDimensions = cityDimensions*2.0f;

      printf("Creating surface from heightmap.\n");
      mb.init(0, 0);
      mb.translate(cityCenter.x(), cityCenter.y(), cityCenter.z());
      mb.rotate(-90, 1, 0, 0);
      mb.add_plane_heightmap(terrainDimensions.x(), terrainDimensions.z(), heightmap_width-2, heightmap_height-2, heightmap.data(), heightmap_width, heightmap_height);
      mb.get_mesh(surfaceMesh);
      //surfaceMesh.set_mode(GL_LINE_STRIP);

      printf("Creating water plane.\n");
      mb.init(0, 0);
      mb.translate(cityCenter.x(), cityCenter.y()+WATER_LEVEL, cityCenter.z());
      mb.rotate(-90, 1, 0, 0);
      mb.add_plane(terrainDimensions.x(), terrainDimensions.z(), 10, 10);
      mb.get_mesh(waterMesh);
    }




		void debugRender(dynarray<Street> *streetsList, bump_shader &shader, const mat4t &modelToProjection, const mat4t &modelToCamera, vec4 *light_uniforms, const int num_light_uniforms, const int num_lights) {
      //grassMaterial->render(shader, modelToProjection, modelToCamera, light_uniforms, num_light_uniforms, num_lights);
      //surfaceMesh.render();

      roadMaterialLeft->render(shader, modelToProjection, modelToCamera, light_uniforms, num_light_uniforms, num_lights);

      for (int i = 0; i != streetsList->size(); ++i) {
        (*streetsList)[i].roadMeshLeft.render();
      }

      roadMaterialRight->render(shader, modelToProjection, modelToCamera, light_uniforms, num_light_uniforms, num_lights);

      for (int i = 0; i != streetsList->size(); ++i) {
        (*streetsList)[i].roadMeshRight.render();
      }

      pavementMaterial->render(shader, modelToProjection, modelToCamera, light_uniforms, num_light_uniforms, num_lights);

      for (int i = 0; i != streetsList->size(); ++i) {
        (*streetsList)[i].pavementMeshLeft.render();
        (*streetsList)[i].pavementMeshRight.render();
      }

      //waterMaterial->render(shader, modelToProjection, modelToCamera, light_uniforms, num_light_uniforms, num_lights);
      //waterMesh.render();
    }



    /*void debugRender_newShader(dynarray<Street> *streetsList, city_bump_shader &city_shader, bump_shader &shader, const mat4t &modelToProjection, const mat4t &modelToCamera, vec4 *light_uniforms, const int num_light_uniforms, const int num_lights) {
    grassMaterial->render(shader, modelToProjection, modelToCamera, light_uniforms, num_light_uniforms, num_lights);
    surfaceMesh.render();

    roadMaterial->render_road(city_shader, modelToProjection, modelToCamera, light_uniforms, num_light_uniforms, num_lights);

    for (int i = 0; i != streetsList->size(); ++i) {
    (*streetsList)[i].roadMesh.render();
    }

    pavementMaterial->render_road(city_shader, modelToProjection, modelToCamera, light_uniforms, num_light_uniforms, num_lights);

    for (int i = 0; i != streetsList->size(); ++i) {
    (*streetsList)[i].pavementMeshLeft.render();
    (*streetsList)[i].pavementMeshRight.render();
    }

    waterMaterial->render(shader, modelToProjection, modelToCamera, light_uniforms, num_light_uniforms, num_lights);
    waterMesh.render();
    }*/

  };



  dynarray <image *> *CityMesh::imageArray_;

  class CompassCard {
    color_shader *cshader;

  public:
    void init(color_shader *cshader_) {
      cshader = cshader_;
    }

    void render(vec4 *camera_position, vec3 *camera_rotation) {
      mat4t modelToWorld;
      modelToWorld.loadIdentity();

      mat4t cameraToWorld;
      cameraToWorld.loadIdentity();
      cameraToWorld.rotate((*camera_rotation)[1], 0.0f, 1.0f, 0.0f);
      cameraToWorld.rotate(-(*camera_rotation)[0], 1.0f, 0.0f, 0.0f);
      cameraToWorld.translate(0.0f, 0.0f, 5.0f); //camera_position->z());

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