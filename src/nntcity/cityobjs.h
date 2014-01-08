namespace octet {

  class Street{
    public:

    mesh roadMesh;
    mesh pavementMeshLeft;
    mesh pavementMeshRight;
    vec4 points[2];

    Street() {
      memset(points, 0, sizeof(vec4)*2);
    }

    Street(vec4 p1, vec4 p2){
      this->points[0] = p1;
      this->points[1] = p2;
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

    dynarray <StreetIntersection*> streetsIntersections;

    class random randomizer;

    vec4 * debugColors;


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
      stepPartition_(depth, &root);

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

      //printf("Start\n");
      glDisable(GL_DEPTH_TEST);
      debugRenderRect_(s, cameraToWorld, aspectRatio, depth, &root);
      glEnable(GL_DEPTH_TEST);
      //printf("End\n");
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

    private:

    void debugRenderRect_(color_shader *s, mat4t *cameraToWorld, float aspectRatio, unsigned int depth, BSPNode *node) {
      if (depth == -1) return;
      if (!node) return;

      mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, *cameraToWorld);

      s->render(modelToProjection, debugColors[depth]);

      float vertices[] = {
        -node->vertices[0].x(), -node->vertices[0].y(), -node->vertices[0].z(),  
        -node->vertices[1].x(), -node->vertices[1].y(), -node->vertices[1].z(),  
        -node->vertices[2].x(), -node->vertices[2].y(), -node->vertices[2].z(),  
        -node->vertices[3].x(), -node->vertices[3].y(), -node->vertices[3].z()
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


    void stepPartition_(unsigned int depth, BSPNode *b) {
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

        generateStreets(b->left);
        generateStreets(b->right);

      }



      // Heuristic to choose side: random by now
      // TODO Heuristic: choose sides intersected by frustrum
    //  int r = randomizer.get(0, 10);
    //  BSPNode *child = (r % 2 ==0)? b->left: b->right;
      float r0 = randomizer.get(0.0f, 1.0f);
      float r1 = randomizer.get(0.0f, 1.0f);
      if (r0 > 0.5f) {
        stepPartition_(depth - 1, b->left);
      }
      if (r1 > 0.5f) {
        stepPartition_(depth - 1, b->right);
      }
    }

    void generateStreets( BSPNode * node) {
      
      dynarray<Street> localList;

      for(int i=0; i!=4; ++i){
 
        if(!streetAlreadyExists(node->vertices[i],node->vertices[(i==3) ? 0 : i+1])){
          
          solveConflictbetweenStreets(node->vertices[i],node->vertices[(i==3) ? 0 : i+1]);

          Street s1(node->vertices[i],node->vertices[(i==3) ? 0 : i+1]);
          localList.push_back(s1);

        }
 
      }

      for(int m=0; m!=localList.size(); m++){
        streetsList.push_back(localList[m]);
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
}