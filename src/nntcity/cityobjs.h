namespace octet {

  class BSPRoot {
  public:
    vec4 vertices[4];
    BSPRoot *left;
    BSPRoot *right;
  };

  class BSPNode : public BSPRoot {
    //Indicates which vertices are common to other partition
    int common_indices[2]; 
  };

  class City {
  public:
    BSPRoot root;

    City () {
    }

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
    }

    void stepPartition(/* camera frustrum */) {
      
    }

    void iterator() {

    }
  };

  
}