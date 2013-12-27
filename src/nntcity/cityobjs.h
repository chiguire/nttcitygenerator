#include <vector>

namespace octet {

  class StreetSides{
    public:
    vec4 points[2];

    StreetSides(vec4 p1, vec4 p2){
      this->points[0] = p1;
      this->points[1] = p2;
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

    std::vector<StreetSides> streetsList;

    class random randomizer;

    vec4 * debugColors;


    City () {}

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
          StreetSides s1(root.vertices[i],root.vertices[i+1]);
          streetsList.push_back(s1);
        }else{
          StreetSides s1(root.vertices[i],root.vertices[0]);
          streetsList.push_back(s1);
        }
      }

      srand (time(NULL));
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
      debugRenderRect_(s, cameraToWorld, aspectRatio, depth, &root);
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

        StreetSides s1(midpoint,midpoint_opposite);
        streetsList.push_back(s1);

      }



      // Heuristic to choose side: random by now
      // TODO Heuristic: choose sides intersected by frustrum
    //  int r = randomizer.get(0, 10);
    //  BSPNode *child = (r % 2 ==0)? b->left: b->right;

      stepPartition_(depth - 1, b->right);
    }



  }; 
}