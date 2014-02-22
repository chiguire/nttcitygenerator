namespace octet {

  class Street{
    public:
    
    vec4 points[2];
    
    dynarray<vec4> roadMeshLeftPoints;
    dynarray<vec4> roadMeshRightPoints;
    dynarray<vec4> pavementMeshLeftPoints;
    dynarray<vec4> pavementMeshRightPoints;

    dynarray<vec4> roadMeshLeftIntersectedPoints;
    dynarray<vec4> roadMeshRightIntersectedPoints;
    dynarray<vec4> pavementMeshLeftIntersectedPoints;
    dynarray<vec4> pavementMeshRightIntersectedPoints;

    dynarray<unsigned short> roadMeshLeftIntersectedIndices;
    dynarray<unsigned short> roadMeshRightIntersectedIndices;
    dynarray<unsigned short> pavementMeshLeftIntersectedIndices;
    dynarray<unsigned short> pavementMeshRightIntersectedIndices;
    
    float angleCS[2];
    float translatedDistance[2];

    Street() {
      memset(points, 0, sizeof(vec4)*2);
    }

    //Copy constructor that may be maybe modified
    Street(Street &s) {                                                       
      this->points[0] = s.points[0];
      this->points[1] = s.points[1];
    } 

    Street(vec4 p1, vec4 p2){
      this->points[0] = p1;
      this->points[1] = p2;
    }

    bool equalsTo(Street *s2){
      return all(this->points[0] == s2->points[0]) && all(this->points[1] == s2->points[1]);
    }

    void intersectGridStreet(float centerX, float centerY, float separationX, float separationY, int width, int height) {
      dynarray<vec4> polygonInput;
      dynarray<vec4> *polygonVec4 = &roadMeshLeftPoints;
      dynarray<vec4> *polygonResultPoints = &roadMeshLeftIntersectedPoints;
      dynarray<unsigned short> *polygonResultIndices = &roadMeshLeftIntersectedIndices;

      // Road Mesh Left
      if (polygonVec4->size() > 0) {
        polygonInput.reset();
        polygonInput.push_back((*polygonVec4)[0]);
        polygonInput.push_back((*polygonVec4)[1]);
        polygonInput.push_back((*polygonVec4)[5]);
        polygonInput.push_back((*polygonVec4)[4]);
        PolygonIntersections::intersectGrid(polygonInput,
                                            centerX, centerY, 
                                            separationX, separationY, 
                                            width, height, 
                                            *polygonResultPoints, *polygonResultIndices);
      }

      // Road Mesh Right
      polygonVec4 = &roadMeshRightPoints;
      polygonResultPoints = &roadMeshRightIntersectedPoints;
      polygonResultIndices = &roadMeshRightIntersectedIndices;

      if (polygonVec4->size() > 0) {
        polygonInput.reset();
        polygonInput.push_back((*polygonVec4)[0]);
        polygonInput.push_back((*polygonVec4)[1]);
        polygonInput.push_back((*polygonVec4)[5]);
        polygonInput.push_back((*polygonVec4)[4]);
        PolygonIntersections::intersectGrid(polygonInput, centerX, centerY, separationX, separationY, width, height, *polygonResultPoints, *polygonResultIndices);
      }

      // Pavement Mesh Left
      polygonVec4 = &pavementMeshLeftPoints;
      polygonResultPoints = &pavementMeshLeftIntersectedPoints;
      polygonResultIndices = &pavementMeshLeftIntersectedIndices;

      if (polygonVec4->size() > 0) {
        polygonInput.reset();
        polygonInput.push_back((*polygonVec4)[0]);
        polygonInput.push_back((*polygonVec4)[1]);
        polygonInput.push_back((*polygonVec4)[5]);
        polygonInput.push_back((*polygonVec4)[4]);
        PolygonIntersections::intersectGrid(polygonInput, centerX, centerY, separationX, separationY, width, height, *polygonResultPoints, *polygonResultIndices);
      }

      // Pavement Mesh Right
      polygonVec4 = &pavementMeshRightPoints;
      polygonResultPoints = &pavementMeshRightIntersectedPoints;
      polygonResultIndices = &pavementMeshRightIntersectedIndices;

      if (polygonVec4->size() > 0) {
        polygonInput.reset();
        polygonInput.push_back((*polygonVec4)[0]);
        polygonInput.push_back((*polygonVec4)[1]);
        polygonInput.push_back((*polygonVec4)[5]);
        polygonInput.push_back((*polygonVec4)[4]);
        PolygonIntersections::intersectGrid(polygonInput, centerX, centerY, separationX, separationY, width, height, *polygonResultPoints, *polygonResultIndices);
      }
    }
  
  };

  class BuildingArea {
  public:
    vec4 points[4];
    mesh areaMesh;

    BuildingArea() {
      memset(points, 0, sizeof(vec4)*4);
    }

    BuildingArea(vec4 p0, vec4 p1, vec4 p2, vec4 p3) {
      points[0] = p0;
      points[1] = p1;
      points[2] = p2;
      points[3] = p3;
    }

    //Copy constructor that may be maybe modified
      BuildingArea(BuildingArea &b) {                                                       
    this->points[0] = b.points[0];
    this->points[1] = b.points[1];
    this->points[2] = b.points[2];
    this->points[3] = b.points[3];
    } 

    bool equalsTo(BuildingArea *b2){
      return all(this->points[0] == b2->points[0]) &&
      all(this->points[1] == b2->points[1]) &&
      all(this->points[2] == b2->points[2]) &&
      all(this->points[3] == b2->points[3]);
    }
  };


  class StreetIntersection{
  public:
    dynarray<Street*> streets;
    vec4 point;

    StreetIntersection(){}

    StreetIntersection(Street* s1, Street* s2, vec4 p){
      this->streets.push_back(s1);
      this->streets.push_back(s2);
      this->point = p;
    }

    bool containsStreet(Street* s1){

      for(int i=0; i!=this->streets.size(); ++i){
        for(int j=0; j!=2; ++j){
          for(int k=0; k!=2; ++k){

            if(all(streets[i]->points[j] == s1->points[k]) && 
                all(streets[i]->points[(j==1) ? 0 : j+1] == s1->points[(k==1) ? 0 : k+1])){
              return true;
            }

          }
        } 
      }

      return false;
      
    }
  };


  class BSPNode {
  public:
    vec4 vertices[4];
    BSPNode *left;
    BSPNode *right;

    BSPNode()
    : left(NULL)
    , right(NULL)
    {
      
    }
  };

  class City {
  public:
    BSPNode root;

    mat4t modelToWorld;

    dynarray<Street> streetsList;
    dynarray<BuildingArea> buildingAreaList;
  
    dynarray <StreetIntersection*> streetsIntersections;

    class random randomizer;

    vec4 * debugColors;

    static const float STREET_WIDTH;
    static const float ROAD_WIDTH;
    static const float PAVEMENT_WIDTH;

    City ()
    :randomizer(time(NULL))
    {}

    static City *createFromRectangle(float width, float height) {
      vec4 vert_[4];

      vert_[0] = vec4(-width/2.0f, 0, -height/2.0f, 1.0f);
      vert_[1] = vec4(-width/2.0f, 0,  height/2.0f, 1.0f);
      vert_[2] = vec4( width/2.0f, 0,  height/2.0f, 1.0f);
      vert_[3] = vec4( width/2.0f, 0, -height/2.0f, 1.0f);

      City *c = new City();
      c->init(vert_);

      return c;
    }

    void init(const vec4 *vertices_) {
      root.vertices[0] = *(vertices_+0);
      root.vertices[1] = *(vertices_+1);
      root.vertices[2] = *(vertices_+2);
      root.vertices[3] = *(vertices_+3);
      root.left = NULL;
      root.right = NULL;
      modelToWorld.loadIdentity();
      
      for(int i=0; i!=4; ++i){
        if(i !=3){
          Street s1(root.vertices[i],root.vertices[i+1]);
          streetsList.push_back(s1);
        }else{
          Street s1(root.vertices[i],root.vertices[0]);
          streetsList.push_back(s1);
        }
      }

      srand (static_cast <unsigned> (time(0)));
    }

    void stepPartition(unsigned int depth/* camera frustrum */) {

      setDebugColors(depth);
      stepPartition_(depth, &root, false);

    }

    void iterator() {

    }

    void setDebugColors(unsigned int depth) {
      
      debugColors = new vec4[depth+1];
      
      for(int i=0; i!= depth+1; ++i){
        vec4 color((float)(rand() % 2),(float)(rand() % 2),(float)(rand() % 2),1.0f);
        debugColors[i] = color;
      }
    }


    void debugRender(color_shader *s, mat4t *cameraToWorld, float aspectRatio, unsigned int depth) {
      glDisable(GL_DEPTH_TEST);
      debugRenderRect_(s, cameraToWorld, aspectRatio, depth, &root);
      glEnable(GL_DEPTH_TEST);
    }

    void printStreets(){
      
      printf("%d Streets:\n\n",streetsList.size());

      for(int i=0; i!= streetsList.size(); ++i){
        printf("%d. (%.2f, %.2f), (%.2f, %.2f).\n",i+1,
        streetsList[i].points[0].x(), streetsList[i].points[0].z(),  
        streetsList[i].points[1].x(), streetsList[i].points[1].z());
      }
    }

    void getDimensions(vec4 &dimensions) {
      vec4 minCoord;
      vec4 maxCoord;

      for (int i = 0; i != 4; i++) {
        vec4 &p = root.vertices[i];

        if (p.x() < minCoord.x()) {
          minCoord[0] = p.x();
        }

        if (p.x() > maxCoord.x()) {
          maxCoord[0] = p.x();
        }

        if (p.y() < minCoord.y()) {
          minCoord[1] = p.y();
        }

        if (p.y() > maxCoord.y()) {
          maxCoord[1] = p.y();
        }
        
        if (p.z() < minCoord.z()) {
          minCoord[2] = p.z();
        }

        if (p.z() > maxCoord.z()) {
          maxCoord[2] = p.z();
        }
      }

      float x = maxCoord[0] - minCoord[0];
      float y = maxCoord[1] - minCoord[1];
      float z = maxCoord[2] - minCoord[2];

      dimensions[0] = x;
      dimensions[1] = y;
      dimensions[2] = z;
      dimensions[3] = 1.0f;
    }

    void getCenter(vec4 &center) {
      //average 4 root points

      center[0] = (root.vertices[0].x() + root.vertices[1].x() + root.vertices[2].x() + root.vertices[3].x())/4.0f;
      center[1] = (root.vertices[0].y() + root.vertices[1].y() + root.vertices[2].y() + root.vertices[3].y())/4.0f;
      center[2] = (root.vertices[0].z() + root.vertices[1].z() + root.vertices[2].z() + root.vertices[3].z())/4.0f;
      center[3] = (root.vertices[0].w() + root.vertices[1].w() + root.vertices[2].w() + root.vertices[3].w())/4.0f;
    }

    void calculateIntersections(){
      for(int i=0; i!= streetsList.size(); ++i){
        for(int j=0; j!=streetsList.size(); ++j){
          for(int k=0; k!=2; ++k){
            for(int l=0; l!=2; ++l){
              //If has any point in common
              if( all(streetsList[i].points[k] == streetsList[j].points[l]) ){
                //If it is not the same street
                if( !all(streetsList[i].points[(k==1) ? 0 : k+1] == streetsList[j].points[(l==1) ? 0 : l+1])){
                  addIntersection(&streetsList[i],&streetsList[j],streetsList[i].points[k]);
                }
              }
            }
          }
        }
      }
    }

    int getStreetsIndex(Street *st){
      for(int i=0; i!= streetsList.size(); ++i){
        if(all(streetsList[i].points[0] == st->points[0]) && all(streetsList[i].points[1] == st->points[1])){
            return i;
        }
      }
      return -1;
    }

    void calculateMeshesIntersections(){

      unsigned int streetsListSize = streetsList.size();
      
      dynarray<bool> checkIntersections(streetsListSize*streetsListSize);
      for (int i = 0; i != streetsListSize*streetsListSize; i++) {
        checkIntersections[i] = false;
      }

      int index = 0;

      for(int i=index; i!= streetsIntersections.size(); ++i){
        StreetIntersection *streetInt = streetsIntersections[i];
        int numStreets = streetInt->streets.size();

        for(int j = 0; j != numStreets; ++j){
          Street *street1 = streetInt->streets[j];

          for(int k = 0; k != numStreets; ++k){
            Street *street2 = streetInt->streets[k];

            if(!(street1->equalsTo(street2))){
              //----------------------We calculate the distance to translate each pair of streets from the intersection point----------------------

              //We obtain two pair of streets
              Street *streetsToModify [2];
              streetsToModify[0] = street1;
              streetsToModify[1] = street2;

              //We obtain their defining vectors setting the origin as the intersection point
              vec4 streetVectors[2];

              for (int x = 0; x != 2; ++x){
                for (int y = 0; y != 2; ++y){
                  if (all(streetInt->point == streetsToModify[x]->points[y])){
                    streetVectors[x] = streetsToModify[x]->points[(y==1) ? 0 : y+1] - streetInt->point;
                  }
                }
              } 

              float operation = dot(streetVectors[0], streetVectors[1]) / (streetVectors[0].length()*streetVectors[1].length());

              if (operation <= -0.99f) {
                operation = -1.0f;
              }

              if (operation > 0.99f) {
                operation = 1.0f;
              }

              float angleBetweenStreets = acos(operation);
              int intAngleBetweenStreets = (int)(angleBetweenStreets*(180.0f/3.14159265359f));

              if (intAngleBetweenStreets != 180) {

                float exteriorPavementDistance = (STREET_WIDTH / 2) / sin(angleBetweenStreets/2);

                float interiorPavementDistance = ((STREET_WIDTH / 2) - PAVEMENT_WIDTH) / sin(angleBetweenStreets/2);

                float exteriorRoadDistance = (ROAD_WIDTH / 2) / sin(angleBetweenStreets/2);

                //------------------------------We calculate the coordinates of the new point of the streets---------------------------------
                float angleStreetsCS[2];  

                for(int p = 0; p != sizeof(streetVectors) / sizeof(streetVectors[0]); ++p){
                  if (streetVectors[p].x() != 0.0f) {
                    angleStreetsCS[p] = atan(streetVectors[p].z() / streetVectors[p].x());
                  } else if (streetVectors[p].z() > 0.0f){
                    angleStreetsCS[p] = 3.14159265359f / 2; // 90 º
                  } else {
                    angleStreetsCS[p] = 3*(3.14159265359f / 2); // 270 º
                  }
                }

                //Change the angle to be in a 360º basis
                for (int q = 0; q != sizeof(streetVectors) / sizeof(streetVectors[0]); ++q){
                  if (streetVectors[q].x() < 0.0f) {
                    angleStreetsCS[q] = 3.14159265359f + angleStreetsCS[q];
                  }

                  if (streetVectors[q].x() > 0.0f && streetVectors[q].z() < 0.0f) {
                    angleStreetsCS[q] = 2 * (3.14159265359f) + angleStreetsCS[q];
                  }
                }

                //We get the smallest street angle and we sum or substract angleBetweenStreets/2 to get the distance vector angle
                float resultingAngle = 0.0f;
                float tempAngle = 0.0f;
                float angleDif = 0.0f;

                if (angleStreetsCS[0] < angleStreetsCS[1]) {
                  
                  angleDif = angleStreetsCS[1] - angleStreetsCS[0];

                  if (angleDif < 3.14159265359f) {
                    resultingAngle = angleStreetsCS[0] + (angleBetweenStreets/2);
                  } else {
                    resultingAngle = angleStreetsCS[0] - (angleBetweenStreets/2);
                  }
                } else {
                  angleDif = angleStreetsCS[0] - angleStreetsCS[1];

                  if (angleDif < 3.14159265359f) {
                    resultingAngle = angleStreetsCS[1] + (angleBetweenStreets/2);
                  } else {
                    resultingAngle = angleStreetsCS[1] - (angleBetweenStreets/2);
                  }
                }

                //We calculate and store the points that define the meshes

                //We obtain the street vectors as v1 - v0
                vec4 streetVectorsStandard[2];
                streetVectorsStandard[0] = streetsToModify[0]->points[1] - streetsToModify[0]->points[0];
                streetVectorsStandard[1] = streetsToModify[1]->points[1] - streetsToModify[1]->points[0];

                for (int z = 0; z != 2; ++z) {
                  Street *streetToModify1 = streetsToModify[z];
                  vec4 *street1VectorStandard = &streetVectorsStandard[z];

                  for (int w = 0; w != 2; ++w) {
                    vec4 &point = streetToModify1->points[w];
                    if (all(streetInt->point == point)) {
                 
                      vec4 exteriorPointPavement (point[0] + exteriorPavementDistance * cos(resultingAngle),0,
                         point[2] + exteriorPavementDistance * sin(resultingAngle),1);

                      vec4 interiorPointPavement (streetToModify1->points[w][0] + interiorPavementDistance * cos(resultingAngle),0,
                        point[2] + interiorPavementDistance * sin(resultingAngle),1);

                      vec4 exteriorPointRoad (streetToModify1->points[w][0] + exteriorRoadDistance * cos(resultingAngle),0,
                        point[2] + exteriorRoadDistance * sin(resultingAngle),1);

                      //We obtain the Cross Product to determine if the points belong to the right of the left road & pavement meshes
                      vec4 exteriorPointVector = exteriorPointPavement - streetToModify1->points[0];

                      exteriorPointVector[1] =  -exteriorPointVector[2]; 
                      (*street1VectorStandard)[1] = -(*street1VectorStandard)[2];

                      float crossProductResult = (street1VectorStandard->x() *exteriorPointVector.y()) - (street1VectorStandard->y() * exteriorPointVector.x()); 
                      //ROAD & PAVEMENT MESHES CALCULATION

                      if (crossProductResult < 0 && !checkIntersections[getStreetsIndex(streetToModify1)+getStreetsIndex(streetsToModify[(z==1) ? 0 : z+1])*streetsListSize]) {

                        streetToModify1->roadMeshRightPoints.push_back(vec4(streetInt->point.x(),0.02f,streetInt->point.z(),streetInt->point.w()));
                        streetToModify1->roadMeshRightPoints.push_back(vec4(exteriorPointRoad.x(),0.02f,exteriorPointRoad.z(),exteriorPointRoad.w()));
                        streetToModify1->roadMeshRightPoints.push_back(vec4(exteriorPointRoad.x(),-0.02f,exteriorPointRoad.z(),exteriorPointRoad.w()));
                        streetToModify1->roadMeshRightPoints.push_back(vec4(streetInt->point.x(),-0.02f,streetInt->point.z(),streetInt->point.w()));

                        streetToModify1->pavementMeshRightPoints.push_back(vec4(interiorPointPavement.x(),0.04f,interiorPointPavement.z(),interiorPointPavement.w()));
                        streetToModify1->pavementMeshRightPoints.push_back(vec4(exteriorPointPavement.x(),0.04f,exteriorPointPavement.z(),exteriorPointPavement.w()));
                        streetToModify1->pavementMeshRightPoints.push_back(vec4(exteriorPointPavement.x(),-0.04f,exteriorPointPavement.z(),exteriorPointPavement.w()));
                        streetToModify1->pavementMeshRightPoints.push_back(vec4(interiorPointPavement.x(),-0.04f,interiorPointPavement.z(),interiorPointPavement.w())); 

                        checkIntersections[getStreetsIndex(streetToModify1)+getStreetsIndex(streetsToModify[(z==1) ? 0 : z+1])*streetsListSize] = true;

                      } else if(crossProductResult > 0 && !checkIntersections[getStreetsIndex(streetToModify1)+getStreetsIndex(streetsToModify[(z==1) ? 0 : z+1])*streetsListSize]){

                        streetToModify1->roadMeshLeftPoints.push_back(vec4(streetsIntersections[i]->point.x(),0.02f,streetsIntersections[i]->point.z(),streetsIntersections[i]->point.w()));
                        streetToModify1->roadMeshLeftPoints.push_back(vec4(exteriorPointRoad.x(),0.02f,exteriorPointRoad.z(),exteriorPointRoad.w()));
                        streetToModify1->roadMeshLeftPoints.push_back(vec4(exteriorPointRoad.x(),-0.02f,exteriorPointRoad.z(),exteriorPointRoad.w()));
                        streetToModify1->roadMeshLeftPoints.push_back(vec4(streetsIntersections[i]->point.x(),-0.02f,streetsIntersections[i]->point.z(),streetsIntersections[i]->point.w()));

                        streetToModify1->pavementMeshLeftPoints.push_back(vec4(interiorPointPavement.x(),0.04f,interiorPointPavement.z(),interiorPointPavement.w()));
                        streetToModify1->pavementMeshLeftPoints.push_back(vec4(exteriorPointPavement.x(),0.04f,exteriorPointPavement.z(),exteriorPointPavement.w()));
                        streetToModify1->pavementMeshLeftPoints.push_back(vec4(exteriorPointPavement.x(),-0.04f,exteriorPointPavement.z(),exteriorPointPavement.w()));
                        streetToModify1->pavementMeshLeftPoints.push_back(vec4(interiorPointPavement.x(),-0.04f,interiorPointPavement.z(),interiorPointPavement.w())); 

                        checkIntersections[getStreetsIndex(streetToModify1)+getStreetsIndex(streetsToModify[(z==1) ? 0 : z+1])*streetsListSize] = true;
                      }
                    }
                  } 
                }
              }
            }
          }
        }
      }
      generatePointsEmptyRoadMeshes();
      generatePointsIncompleteMeshes();
      printMeshesPoints();
    } 

    //generates 4 or 8 additional points for the Road Meshes that just have 4 points
    void generatePointsEmptyRoadMeshes(){
      float exteriorPavementDistance = (STREET_WIDTH / 2);
      float interiorPavementDistance = ((STREET_WIDTH / 2) - PAVEMENT_WIDTH);

      for(int i=0; i!= streetsList.size(); ++i){

        if(streetsList[i].roadMeshLeftPoints.size() == 0  || streetsList[i].roadMeshRightPoints.size() == 0 ){

          Street *st = &streetsList[i];

          vec4 streetVector = st->points[1] - st->points[0];

          vec4 perpendicularVector;

          dynarray<vec4>* roadMeshToApplyChanges;
          dynarray<vec4>* pavementMeshToApplyChanges;

          if(streetsList[i].roadMeshLeftPoints.size() == 0 ){
            perpendicularVector = vec4(streetVector[2],streetVector[1],-streetVector[0],1);
            roadMeshToApplyChanges = &(st->roadMeshLeftPoints);
            pavementMeshToApplyChanges = &(st->pavementMeshLeftPoints);
          }
          
          if(streetsList[i].roadMeshRightPoints.size() == 0){
            perpendicularVector = vec4(-streetVector[2],streetVector[1],streetVector[0],1);
            roadMeshToApplyChanges = &(st->roadMeshRightPoints);
            pavementMeshToApplyChanges = &(st->pavementMeshRightPoints);
          }

          float anglePerpendicularVector = 0.0f;
        
          //We obtain the angle of the perpendicular vector to the street

          if(perpendicularVector.x() != 0.0f){
            anglePerpendicularVector = atan(perpendicularVector.z()/perpendicularVector.x());
          }else if(perpendicularVector.z() > 0.0f){
            anglePerpendicularVector = 3.14159265359f / 2; // 90 º
          }else{
            anglePerpendicularVector = 3*(3.14159265359f / 2); // 270 º
          }

          //Change the angle to be in a 360º basis
          if(perpendicularVector.x() < 0.0f){
            anglePerpendicularVector = 3.14159265359f + anglePerpendicularVector;
          }

          if(perpendicularVector.x() > 0.0f && perpendicularVector.z() < 0.0f ){
            anglePerpendicularVector = 2 * (3.14159265359f) + anglePerpendicularVector;
          }

          vec4 exteriorPointRoad1 (st->points[0].x() + (ROAD_WIDTH / 2) * cos(anglePerpendicularVector),0,
            st->points[0].z()  + (ROAD_WIDTH / 2) * sin(anglePerpendicularVector),1);

          vec4 exteriorPointRoad2 (st->points[1].x() + (ROAD_WIDTH / 2) * cos(anglePerpendicularVector),0,
            st->points[1].z() + (ROAD_WIDTH / 2) * sin(anglePerpendicularVector),1);

          roadMeshToApplyChanges->push_back(vec4(st->points[0].x(),0.02f,st->points[0].z(),st->points[0].w()));
          roadMeshToApplyChanges->push_back(vec4(exteriorPointRoad1.x(),0.02f,exteriorPointRoad1.z(),exteriorPointRoad1.w()));
          roadMeshToApplyChanges->push_back(vec4(exteriorPointRoad1.x(),-0.02f,exteriorPointRoad1.z(),exteriorPointRoad1.w()));
          roadMeshToApplyChanges->push_back(vec4(st->points[0].x(),-0.02f,st->points[0].z(),st->points[0].w()));  

          roadMeshToApplyChanges->push_back(vec4(st->points[1].x(),0.02f,st->points[1].z(),st->points[1].w()));
          roadMeshToApplyChanges->push_back(vec4(exteriorPointRoad2.x(),0.02f,exteriorPointRoad2.z(),exteriorPointRoad2.w()));
          roadMeshToApplyChanges->push_back(vec4(exteriorPointRoad2.x(),-0.02f,exteriorPointRoad2.z(),exteriorPointRoad2.w()));
          roadMeshToApplyChanges->push_back(vec4(st->points[1].x(),-0.02f,st->points[1].z(),st->points[1].w())); 

          

          vec4 exteriorPointPavement (st->points[0].x() + (ROAD_WIDTH / 2) * cos(anglePerpendicularVector),0,
            st->points[0].z() + exteriorPavementDistance * sin(anglePerpendicularVector),1);

          vec4 interiorPointPavement (st->points[1].x() + (ROAD_WIDTH / 2) * cos(anglePerpendicularVector),0,
            st->points[1].z() + interiorPavementDistance * sin(anglePerpendicularVector),1);
          
          pavementMeshToApplyChanges->push_back(vec4(interiorPointPavement.x(),0.04f,interiorPointPavement.z(),interiorPointPavement.w()));
          pavementMeshToApplyChanges->push_back(vec4(exteriorPointPavement.x(),0.04f,exteriorPointPavement.z(),exteriorPointPavement.w()));
          pavementMeshToApplyChanges->push_back(vec4(exteriorPointPavement.x(),-0.04f,exteriorPointPavement.z(),exteriorPointPavement.w()));
          pavementMeshToApplyChanges->push_back(vec4(interiorPointPavement.x(),-0.04f,interiorPointPavement.z(),interiorPointPavement.w())); 
        }
      }
    }

    //generates 4 additional points for the Pavement Meshes that just have 4 points
    void generatePointsIncompleteMeshes(){

      for(int i=0; i!= streetsList.size(); ++i){
          if(streetsList[i].pavementMeshLeftPoints.size() == 4 || streetsList[i].pavementMeshRightPoints.size() == 4 ||
            streetsList[i].roadMeshLeftPoints.size() == 4 || streetsList[i].roadMeshRightPoints.size() == 4){
            
            Street *st = &streetsList[i];
            
            //We determine which point of the street is the closest one to the Incomplete Mesh
            
            vec4 incompleteMeshPoint;            

            if(st->pavementMeshLeftPoints.size() == 4){
              incompleteMeshPoint = st->pavementMeshLeftPoints[0];
            }else{
              incompleteMeshPoint = st->pavementMeshRightPoints[0];
            }

            
            vec4 streetPoint;
            float distance = 0.0;            

            for(int j=0; j!= sizeof(st->points)/sizeof(st->points[0]); ++j){

             float dis = sqrt( (incompleteMeshPoint.x() - st->points[j].x()) * (incompleteMeshPoint.x() - st->points[j].x()) + 
                               (incompleteMeshPoint.z() - st->points[j].z()) * (incompleteMeshPoint.z() - st->points[j].z()) );


              if(dis > distance){
                distance = dis;
                streetPoint = st->points[j];
              }
            }


            vec4 streetVector = st->points[1] - st->points[0];
              
            vec4 perpendicularVector;

            if(st->pavementMeshLeftPoints.size() == 4){
              perpendicularVector = vec4(streetVector[2],streetVector[1],-streetVector[0],1);
            }else{
              perpendicularVector = vec4(-streetVector[2],streetVector[1],streetVector[0],1);
            }

            float anglePerpendicularVector = 0.0f;
            //We obtain the angle of the perpendicular vector to the street
                            
            if(perpendicularVector.x() != 0.0f){
              anglePerpendicularVector = atan(perpendicularVector.z()/perpendicularVector.x());
            }else if(perpendicularVector.z() > 0.0f){
              anglePerpendicularVector = 3.14159265359f / 2; // 90 º
            }else{
              anglePerpendicularVector = 3*(3.14159265359f / 2); // 270 º
            }

          //Change the angle to be in a 360º basis
            if(perpendicularVector.x() < 0.0f){
              anglePerpendicularVector = 3.14159265359f + anglePerpendicularVector;
            }

            if(perpendicularVector.x() > 0.0f && perpendicularVector.z() < 0.0f ){
              anglePerpendicularVector = 2 * (3.14159265359f) + anglePerpendicularVector;
            } 
            
                            
            vec4 exteriorPointPavement (streetPoint[0] + (STREET_WIDTH / 2) * cos(anglePerpendicularVector),0,
                         streetPoint[2] + (STREET_WIDTH / 2) * sin(anglePerpendicularVector),1);

            vec4 interiorPointPavement (streetPoint[0] + ((STREET_WIDTH / 2) - PAVEMENT_WIDTH) * cos(anglePerpendicularVector),0,
              streetPoint[2] + ((STREET_WIDTH / 2) - PAVEMENT_WIDTH) * sin(anglePerpendicularVector),1);

            vec4 exteriorPointRoad (streetPoint[0] + (ROAD_WIDTH / 2) * cos(anglePerpendicularVector),0,
              streetPoint[2] + (ROAD_WIDTH / 2) * sin(anglePerpendicularVector),1);

            
            if(st->pavementMeshRightPoints.size() == 4){

              st->pavementMeshRightPoints.push_back(vec4(interiorPointPavement.x(),0.04f,interiorPointPavement.z(),interiorPointPavement.w()));
              st->pavementMeshRightPoints.push_back(vec4(exteriorPointPavement.x(),0.04f,exteriorPointPavement.z(),exteriorPointPavement.w()));
              st->pavementMeshRightPoints.push_back(vec4(exteriorPointPavement.x(),-0.04f,exteriorPointPavement.z(),exteriorPointPavement.w()));
              st->pavementMeshRightPoints.push_back(vec4(interiorPointPavement.x(),-0.04f,interiorPointPavement.z(),interiorPointPavement.w())); 

            }else if(st->pavementMeshLeftPoints.size() == 4){

              st->pavementMeshLeftPoints.push_back(vec4(interiorPointPavement.x(),0.04f,interiorPointPavement.z(),interiorPointPavement.w()));
              st->pavementMeshLeftPoints.push_back(vec4(exteriorPointPavement.x(),0.04f,exteriorPointPavement.z(),exteriorPointPavement.w()));
              st->pavementMeshLeftPoints.push_back(vec4(exteriorPointPavement.x(),-0.04f,exteriorPointPavement.z(),exteriorPointPavement.w()));
              st->pavementMeshLeftPoints.push_back(vec4(interiorPointPavement.x(),-0.04f,interiorPointPavement.z(),interiorPointPavement.w())); 

            }

            if(st->roadMeshRightPoints.size() == 4){

              st->roadMeshRightPoints.push_back(vec4(streetPoint.x(),0.02f,streetPoint.z(),streetPoint.w()));
              st->roadMeshRightPoints.push_back(vec4(exteriorPointRoad.x(),0.02f,exteriorPointRoad.z(),exteriorPointRoad.w()));
              st->roadMeshRightPoints.push_back(vec4(exteriorPointRoad.x(),-0.02f,exteriorPointRoad.z(),exteriorPointRoad.w()));
              st->roadMeshRightPoints.push_back(vec4(streetPoint.x(),-0.02f,streetPoint.z(),streetPoint.w())); 

            }else if(st->roadMeshLeftPoints.size() == 4){

              st->roadMeshLeftPoints.push_back(vec4(streetPoint.x(),0.02f,streetPoint.z(),streetPoint.w()));
              st->roadMeshLeftPoints.push_back(vec4(exteriorPointRoad.x(),0.02f,exteriorPointRoad.z(),exteriorPointRoad.w()));
              st->roadMeshLeftPoints.push_back(vec4(exteriorPointRoad.x(),-0.02f,exteriorPointRoad.z(),exteriorPointRoad.w()));
              st->roadMeshLeftPoints.push_back(vec4(streetPoint.x(),-0.02f,streetPoint.z(),streetPoint.w())); 

            }


                
          }
      }
    }

    void printMeshesPoints(){
      printf("%d Streets:\n\n",streetsList.size());

      for(int i=0; i!= streetsList.size(); ++i){
        printf("%d. (%.2f, %.2f), (%.2f, %.2f).\n",i+1,
          streetsList[i].points[0].x(), streetsList[i].points[0].z(),  
          streetsList[i].points[1].x(), streetsList[i].points[1].z());

        printf("\n*Right Road points:\n");
        for(int j=0; j!= streetsList[i].roadMeshRightPoints.size(); ++j){
          printf("  %d. (%.2f, %.2f, %.2f)\n",j,streetsList[i].roadMeshRightPoints[j].x(), streetsList[i].roadMeshRightPoints[j].y(), streetsList[i].roadMeshRightPoints[j].z());
        }
        printf("\n*Left Road points:\n");
        for(int k=0; k!= streetsList[i].roadMeshLeftPoints.size(); ++k){
          printf("  %d. (%.2f, %.2f, %.2f)\n",k,streetsList[i].roadMeshLeftPoints[k].x(), streetsList[i].roadMeshLeftPoints[k].y(), streetsList[i].roadMeshLeftPoints[k].z());
        }

       /* printf("\n*Right Pavement points:\n");
        for(int j=0; j!= streetsList[i].pavementMeshRightPoints.size(); ++j){
          printf("  %d. (%.2f, %.2f, %.2f)\n",j,streetsList[i].pavementMeshRightPoints[j].x(), streetsList[i].pavementMeshRightPoints[j].y(), streetsList[i].pavementMeshRightPoints[j].z());
        }
        printf("\n*Left Pavement points:\n");
        for(int k=0; k!= streetsList[i].pavementMeshLeftPoints.size(); ++k){
          printf("  %d. (%.2f, %.2f, %.2f)\n",k,streetsList[i].pavementMeshLeftPoints[k].x(), streetsList[i].pavementMeshLeftPoints[k].y(), streetsList[i].pavementMeshLeftPoints[k].z());
        } */
        printf("\n");
      }
    }

    void printIntersections(){
      
      printf("\n\n%d Intersections:\n\n",streetsIntersections.size());

      for(int i=0; i!= streetsIntersections.size(); ++i){
       printf("Point (%.2f, %.2f): \n",streetsIntersections[i]->point.x(),streetsIntersections[i]->point.z());
        for(int j=0; j!= streetsIntersections[i]->streets.size(); ++j){
          printf("s%d. (%.2f, %.2f), (%.2f, %.2f).\n",j+1,
          streetsIntersections[i]->streets[j]->points[0].x(), streetsIntersections[i]->streets[j]->points[0].z(),  
          streetsIntersections[i]->streets[j]->points[1].x(), streetsIntersections[i]->streets[j]->points[1].z());
        }
      printf("\n");
      }
    }

  void calculateBuildingsAreas(float scale) {
    calculateBuildingsAreas_(&root, scale);
  }


    private:

    void debugRenderRect_(color_shader *s, mat4t *cameraToWorld, float aspectRatio, unsigned int depth, BSPNode *node) {
      if (depth == -1) return;
      if (!node) return;

      mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, *cameraToWorld);

      s->render(modelToProjection, debugColors[depth]);

      float vertices[] = {
        node->vertices[0].x(), node->vertices[0].y(), node->vertices[0].z(),  
        node->vertices[1].x(), node->vertices[1].y(), node->vertices[1].z(),  
        node->vertices[2].x(), node->vertices[2].y(), node->vertices[2].z(),  
        node->vertices[3].x(), node->vertices[3].y(), node->vertices[3].z()
      };

   /*   printf("Rendering vertices: (%.2f, %.2f), (%.2f, %.2f), (%.2f, %.2f,), (%.2f, %.2f).\n",
        node->vertices[0].x(), node->vertices[0].z(),  
        node->vertices[1].x(), node->vertices[1].z(),  
        node->vertices[2].x(), node->vertices[2].z(),  
        node->vertices[3].x(), node->vertices[3].z()); */

      glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void *)vertices);
      glEnableVertexAttribArray(attribute_pos);

      glDrawArrays(GL_LINE_LOOP, 0, 4);

      debugRenderRect_(s, cameraToWorld, aspectRatio, depth-1, node->left);
      debugRenderRect_(s, cameraToWorld, aspectRatio, depth-1, node->right);
    }

   
    //We apply the heuristic: maximum value of the sum of the opposite sides, and then we yield one of those sides
    
    int getSideToMakePartition( BSPNode * b ) {

      float sideLengthSum1 = abs(b->vertices[0] - b->vertices[1]).length() + abs(b->vertices[3] - b->vertices[2]).length();

      float sideLengthSum2 = abs(b->vertices[0] - b->vertices[3]).length() + abs(b->vertices[1] - b->vertices[2]).length();

      return (sideLengthSum1 > sideLengthSum2)? 0: 1;

    }

  void calculateBuildingsAreas_(BSPNode *b, float scale) {

    if (b->right ) {
      calculateBuildingsAreas_(b->right, scale);
    }

    if (b->left) {
      calculateBuildingsAreas_(b->left, scale);
    } else {
      vec4 v0 = b->vertices[0];
      vec4 v1 = b->vertices[1];
      vec4 v2 = b->vertices[2];
      vec4 v3 = b->vertices[3];

      vec4 v0t = b->vertices[0]*scale;
      vec4 v1t = b->vertices[1]*scale;
      vec4 v2t = b->vertices[2]*scale;
      vec4 v3t = b->vertices[3]*scale;
      
      // find the center point of quadrangle 
      float nX = (v0.x() + v1.x() + v2.x() + v3.x())/4;
      float nZ = (v0.z() + v1.z() + v2.z() + v3.z())/4;
      
      // find the center point of scaled quadrangle
      float nXt = (v0t.x() + v1t.x() + v2t.x() + v3t.x())/4;
      float nZt = (v0t.z() + v1t.z() + v2t.z() + v3t.z())/4;
      
      vec4 quad_center = vec4(nX, 0.0f, nZ, 1.0f);
      vec4 quad_center_t = vec4(nXt, 0.0f, nZt, 1.0f);
      vec4 dist_vector = vec4(quad_center.x()-quad_center_t.x(), 0, quad_center.z()-quad_center_t.z(), 1);

      BuildingArea buildingArea = BuildingArea(v0t+dist_vector, v1t+dist_vector, v2t+dist_vector, v3t+dist_vector);

      
      
      BSPNode buildingNodeRoot = BSPNode();
      buildingNodeRoot.vertices[0] = buildingArea.points[0];
      buildingNodeRoot.vertices[1] = buildingArea.points[1];
      buildingNodeRoot.vertices[2] = buildingArea.points[2];
      buildingNodeRoot.vertices[3] = buildingArea.points[3];

      stepPartition_(5, &buildingNodeRoot, true);

      printf("-------------------- \n");
      printf("Building Areas points \n");
      printf(" v0 - %f, %f, %f, %f \n", v0.x(), v0.y(), v0.z(), v0.w());
      printf(" v1 - %f, %f, %f, %f \n", v1.x(), v1.y(), v1.z(), v1.w());
      printf(" v1 - %f, %f, %f, %f \n", v2.x(), v2.y(), v2.z(), v2.w());
      printf(" v1 - %f, %f, %f, %f \n", v3.x(), v3.y(), v3.z(), v3.w());
    }
  }



    void stepPartition_(unsigned int depth, BSPNode *b, bool noStreet) {
      if (depth == 0) return;

      if (!b->right || !b->left) { // It was a leaf node, expand it


        int side_index = getSideToMakePartition(b);


        //Opposite side
        int opposite_side_index = (side_index+2)%4;
        vec4 &opposite_side_vertex_a = b->vertices[opposite_side_index];
        vec4 &opposite_side_vertex_b = b->vertices[(opposite_side_index+1)%4];

        vec4 &side_vertex_a = b->vertices[side_index];
        vec4 &side_vertex_b = b->vertices[(side_index+1)%4];

        //Calculate mid-points
        vec4 midpoint(side_vertex_a.x() + (side_vertex_b.x() - side_vertex_a.x())*0.5f,
                      side_vertex_a.y() + (side_vertex_b.y() - side_vertex_a.y())*0.5f,
                      side_vertex_a.z() + (side_vertex_b.z() - side_vertex_a.z())*0.5f,
                      1.0f);

        vec4 midpoint_opposite(opposite_side_vertex_a.x() + (opposite_side_vertex_b.x() - opposite_side_vertex_a.x())*0.5f,
                      opposite_side_vertex_a.y() + (opposite_side_vertex_b.y() - opposite_side_vertex_a.y())*0.5f,
                      opposite_side_vertex_a.z() + (opposite_side_vertex_b.z() - opposite_side_vertex_a.z())*0.5f,
                      1.0f);

        b->left = new BSPNode();
        b->right = new BSPNode();

        //Put resulting vertices in the same positions as its parent
        b->left->vertices[0] = side_vertex_a;
        b->left->vertices[1] = midpoint;
        b->left->vertices[2] = midpoint_opposite;
        b->left->vertices[3] = opposite_side_vertex_b;

        b->right->vertices[0] = opposite_side_vertex_a;
        b->right->vertices[1] = midpoint_opposite;
        b->right->vertices[2] = midpoint;
        b->right->vertices[3] = side_vertex_b;

        generateStreets(b->left, noStreet);
        generateStreets(b->right, noStreet);

      }


      //stepPartition_(depth - 1, b->left);

      // Heuristic to choose side: random by now
      // TODO Heuristic: choose sides intersected by frustrum

      
      float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

      if (r >= 0.5f) {
        stepPartition_(depth - 1, b->left, noStreet);
      }
      if (r < 0.5f) {
        stepPartition_(depth - 1, b->right, noStreet);
      }
      
    }

    void generateStreets( BSPNode * node, bool noStreet) {
      
      dynarray<Street> localList;

      for(int i=0; i!=4; ++i){
 
        if(!streetAlreadyExists(node->vertices[i],node->vertices[(i==3) ? 0 : i+1])){
          
          solveConflictbetweenStreets(node->vertices[i],node->vertices[(i==3) ? 0 : i+1]);

          Street s1(node->vertices[i],node->vertices[(i==3) ? 0 : i+1]);
          localList.push_back(s1);

        }
 
      }

     if (!noStreet){
      for(int m=0; m!=localList.size(); m++){
      streetsList.push_back(localList[m]);
      }
    }
    else {
      buildingAreaList.push_back(BuildingArea(node->vertices[0], node->vertices[1], node->vertices[2], node->vertices[3]));
    }
    }

    bool streetAlreadyExists(vec4 sp1Son, vec4 sp2Son){
      
      vec4 streenSonPoints[2];
      streenSonPoints[0] = sp1Son;
      streenSonPoints[1] = sp2Son;

      for(int i=0; i!=streetsList.size(); ++i){
        for(int j=0; j!=2; ++j){
          for(int k=0; k!=2; ++k){

            if(all(streetsList[i].points[j] == streenSonPoints[k]) && 
                all(streetsList[i].points[(j==1) ? 0 : j+1] == streenSonPoints[(k==1) ? 0 : k+1])){
              return true;
            }

          }
        } 
      }

      return false;
      
    }

    void solveConflictbetweenStreets(vec4 sp1Son, vec4 sp2Son) {
      
      vec4 streenSonPoints[2];
      streenSonPoints[0] = sp1Son;
      streenSonPoints[1] = sp2Son;

      int indexToDelete=-1;

      for(int i=0; i!=streetsList.size(); ++i){
        for(int j=0; j!=2; ++j){
          for(int k=0; k!=2; ++k){

            if(all(streetsList[i].points[j] == streenSonPoints[k])){

              vec4 midpoint(streetsList[i].points[j].x() + (streetsList[i].points[(j==1) ? 0 : j+1].x() - streetsList[i].points[j].x())*0.5f,
                       streetsList[i].points[j].y() + (streetsList[i].points[(j==1) ? 0 : j+1].y() - streetsList[i].points[j].y())*0.5f,
                        streetsList[i].points[j].z() + (streetsList[i].points[(j==1) ? 0 : j+1].z() - streetsList[i].points[j].z())*0.5f,
                        1.0f); 
              
              if(all(midpoint == streenSonPoints[(k==1) ? 0 : k+1])){
                indexToDelete = i;
              }
            }
           }
          }
        } 
 
        if(indexToDelete!=-1){
          streetsList.erase(indexToDelete);
        }
    }

     

    void addIntersection(Street *s1, Street *s2, vec4 p ) 
    {
      
      StreetIntersection* st = new StreetIntersection(s1,s2,p);
      

      for(int i=0; i!= streetsIntersections.size(); ++i){
        //The intersection exists
        if( all( streetsIntersections[i]->point == p ) ){
          if(!streetsIntersections[i]->containsStreet(s2)) { 
            streetsIntersections[i]->streets.push_back(s2);
              /* printf("Add street :(%.2f, %.2f), (%.2f, %.2f) in (%.2f, %.2f).\n",
              s2.points[0].x(), s2.points[0].z(), s2.points[1].x(), s2.points[1].z(),p.x(),p.z()); */
               
          }
          goto outloop; 
        }
      }

      //The intersection does not exist
      
      streetsIntersections.push_back(st);
     /* printf("Add intersection :(%.2f, %.2f), (%.2f, %.2f) -- (%.2f, %.2f), (%.2f, %.2f) in (%.2f, %.2f).\n",
        s1.points[0].x(), s1.points[0].z(), s1.points[1].x(), s1.points[1].z(), s2.points[0].x(), s2.points[0].z(), 
        s2.points[1].x(), s2.points[1].z(),p.x(),p.z());      */

      outloop:;
    }


  

  
  }; 
  
  const float City::STREET_WIDTH = 0.26f;
  const float City::ROAD_WIDTH = 0.20f;
  const float City::PAVEMENT_WIDTH = 0.04;

}