////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// 3D mesh container
//
// mesh builder class for standard meshes.
//

namespace octet {
  class mesh_builder {
    struct vertex { float pos[3]; float normal[3]; float uv[2]; };
    dynarray<vertex, allocator> vertices;
    dynarray<unsigned short, allocator> indices;
    
    struct sphere {
      vec4 center;
      union {
        int children[2];
        int indices[3];
      };
      bool is_leaf;
    };

    dynarray<sphere> spheres;

    // current orientation and position of components
    mat4t matrix;

    // For a cube, add the front face. Matrix transforms are used to add the others.
    void add_front_face(float size) {
      unsigned short cur_vertex = (unsigned short)vertices.size();
      add_vertex(vec4(-size, -size, size, 1), vec4(0, 0, 1, 0), 0, 0);
      add_vertex(vec4(-size,  size, size, 1), vec4(0, 0, 1, 0), 0, 1);
      add_vertex(vec4( size,  size, size, 1), vec4(0, 0, 1, 0), 1, 1);
      add_vertex(vec4( size, -size, size, 1), vec4(0, 0, 1, 0), 1, 0);
      indices.push_back(cur_vertex+0);
      indices.push_back(cur_vertex+1);
      indices.push_back(cur_vertex+2);
      indices.push_back(cur_vertex+0);
      indices.push_back(cur_vertex+2);
      indices.push_back(cur_vertex+3);
    }

    // Add a front face aligned to plane XY, with distance z from origin
    void add_front_face(float x, float y, float z, float xOffset = 0.0f, float yOffset = 0.0f) {
      unsigned short cur_vertex = (unsigned short)vertices.size();
      add_vertex(vec4(-x + xOffset, -y + yOffset, z, 1), vec4(0, 0, 1, 0), 0, 0);
      add_vertex(vec4(-x + xOffset,  y + yOffset, z, 1), vec4(0, 0, 1, 0), 0, 1);
      add_vertex(vec4( x + xOffset,  y + yOffset, z, 1), vec4(0, 0, 1, 0), 1, 1);
      add_vertex(vec4( x + xOffset, -y + yOffset, z, 1), vec4(0, 0, 1, 0), 1, 0);
      indices.push_back(cur_vertex+0);
      indices.push_back(cur_vertex+1);
      indices.push_back(cur_vertex+2);
      indices.push_back(cur_vertex+0);
      indices.push_back(cur_vertex+2);
      indices.push_back(cur_vertex+3);
    }

    void add_face(vec4 v1, vec4 v2, vec4 v3, vec4 v4, vec4 normal){
      unsigned short cur_vertex = (unsigned short)vertices.size();
      add_vertex(v1, normal, 0, 0);
      add_vertex(v2, normal, 0, 1);
      add_vertex(v3, normal, 1, 1);
      add_vertex(v4, normal, 1, 0);
      indices.push_back(cur_vertex+0);
      indices.push_back(cur_vertex+1);
      indices.push_back(cur_vertex+2);
      indices.push_back(cur_vertex+0);
      indices.push_back(cur_vertex+2);
      indices.push_back(cur_vertex+3);
    }

    // add a ring in the x-y plane. Return index of first index
    unsigned add_ring(float radius, const vec4 &normal, unsigned num_vertices, float v, float uvscale) {
      float rnv = 1.0f / num_vertices;
      float angle = 3.1415926536f * 2 * rnv;
      float delta_c = cosf(angle), delta_s = sinf(angle);
      mat4t save_matrix = matrix;
      unsigned first_index = (unsigned)vertices.size();
      float u = 0;
      for (unsigned i = 0; i <= num_vertices; ++i) {
        add_vertex(vec4(radius, 0, 0, 1), normal, u, v);
        matrix.rotateSpecial(delta_c, delta_s, 0, 1);
        u += rnv * uvscale;
      }
      matrix = save_matrix;
      return first_index;
    }

    void add_cone_or_sphere(float radius, float height, unsigned slices, unsigned stacks, float uvscale, bool is_sphere) {
      float rstacks = 1.0f / stacks;

      mat4t save_matrix = matrix;

      // start at the south pole and work up
      matrix.translate(0, 0, -radius);
      float v = 0;
      unsigned prev_ring = 0;
      vec4 cone_normal = vec4(height, 0, radius, 0).normalize();

      if (!is_sphere) {
        // end cap for cone
        unsigned center = add_vertex(vec4(0, 0, 0, 1), vec4(0, 0, -1, 0), 0, 1);
        unsigned cur_ring = add_ring(radius, vec4(0, 0, -1, 0), slices, 0, uvscale);
        for (unsigned j = 0; j != slices; ++j) {
          indices.push_back(center);
          indices.push_back(cur_ring + j);
          indices.push_back(cur_ring + j + 1);
        }
      }

      for (unsigned i = 0; i <= stacks; ++i) {
        float c = cosf(i * rstacks * 3.1415926536f);
        float s = sinf(i * rstacks * 3.1415926536f);
        float ring_radius = is_sphere ? radius * s : radius * (stacks - i) * rstacks;
        float z = is_sphere ? (-radius) * c : height * (stacks - i) * rstacks;
        vec4 normal = is_sphere ? vec4(s, 0, -c, 0) : cone_normal;
        matrix = save_matrix;
        matrix.translate(0, 0, z);
        unsigned cur_ring = add_ring(ring_radius, normal, slices, v, uvscale);
        //printf("%d/%d z=%f r=%f\n", i, stacks, z, ring_radius);
        v += rstacks * radius * uvscale;
        if (i != 0) {
          for (unsigned j = 0; j != slices; ++j) {
            indices.push_back(prev_ring + j);
            indices.push_back(cur_ring + j);
            indices.push_back(cur_ring + j + 1);
            indices.push_back(prev_ring + j);
            indices.push_back(cur_ring + j + 1);
            indices.push_back(prev_ring + j + 1);
          }
        }
        prev_ring = cur_ring;
      }
      matrix = save_matrix;
    }

  public:
    mesh_builder() {
      init();
    }

    void init(int num_vertices=0, int num_indices=0) {
      vertices.resize(0);
      indices.resize(0);
      vertices.reserve(num_vertices);
      indices.reserve(num_indices);
      matrix.loadIdentity();
    }

    // add one vertex to the model
    unsigned add_vertex(const vec4 &pos, const vec4 &normal, float u, float v) {
      vec4 tpos = pos * matrix;
      vec4 tnormal = normal * matrix;
      vertex vtx = { tpos[0], tpos[1], tpos[2], tnormal[0], tnormal[1], tnormal[2], u, v };
      unsigned result = (unsigned)vertices.size();
      vertices.push_back(vtx);
      return result;
    }

    // add one index to the model
    void add_index(unsigned index) {
      indices.push_back(index);
    }

    // add a cube to the model at the current matrix location
    // as in glutSolidCube
    void add_cube(float size) {
      add_front_face(size);
      matrix.rotateY90();
      add_front_face(size);
      matrix.rotateY90();
      add_front_face(size);
      matrix.rotateY90();
      add_front_face(size);
      matrix.rotateY90();

      matrix.rotateX90();
      add_front_face(size);
      matrix.rotateX180();
      add_front_face(size);
      matrix.rotateX90();
    }

    void add_cuboid(float x, float y, float z) {
      add_front_face(x, y, z);
      matrix.rotateY90();
      add_front_face(z, y, x);
      matrix.rotateY90();
      add_front_face(x, y, z);
      matrix.rotateY90();
      add_front_face(z, y, x);
      matrix.rotateY90();

      matrix.rotateX90();
      add_front_face(x, z, y);
      matrix.rotateX180();
      add_front_face(x, z, y);
      matrix.rotateX90();
    }

    void add_cuboid_vertices(dynarray<vec4> *vertices){
      //Up face
      add_face((*vertices)[0], (*vertices)[1], (*vertices)[5], (*vertices)[4], vec4(0, 1, 0, 0));
      //Down face
      add_face((*vertices)[3], (*vertices)[2], (*vertices)[6], (*vertices)[7], vec4(0, -1, 0, 0));
      //Right face
      add_face((*vertices)[2], (*vertices)[6], (*vertices)[5], (*vertices)[1], vec4(1, 0, 0, 0));
      //Left face
      add_face((*vertices)[3], (*vertices)[7], (*vertices)[4], (*vertices)[0], vec4(-1, 0, 0, 0));
    }

    void add_vertices(dynarray<vec4> &vertices_, dynarray<unsigned short> &indices_, dynarray<vec4> &normals_, dynarray<vec2> &uvcoords_,
                      vec4 &cityDimensions, vec4 &cityCenter, float multiplier, float offsetX, float offsetY,
                      unsigned nx, unsigned ny, vec4 *normalMap, unsigned hmx, unsigned hmy, float *heightmap) {
      unsigned short cur_vertex = (unsigned short)vertices.size();

      for (int i = 0; i != vertices_.size(); i++) {
        vec4 &vertex = vertices_[i];

        if (normals_[i].x() == 0.0f && normals_[i].z() == 0.0f) {
          float u_ = (vertex.x()-cityCenter.x()+cityDimensions.x()*0.5f) / (cityDimensions.x());
          float v_ = (vertex.z()-cityCenter.z()+cityDimensions.z()*0.5f) / (cityDimensions.z());
          u_ = (u_*multiplier)+offsetX;
          v_ = ((1-v_)*multiplier)+offsetY;
          add_vertex(vertex, normalMap[nx*(int)floor(v_*ny)+(int)floor(u_*nx)], uvcoords_[i].x(), uvcoords_[i].y());
        } else {
          add_vertex(vertex, normals_[i], 0.0f, 0.0f);
        }
      }
      for (auto i = indices_.begin(); i != indices_.end(); i++) {
        indices.push_back(cur_vertex+*i);
      }
    }

    void add_cuboid_heights(float x, float y, float z, unsigned nz, float *heights = NULL) {
      add_front_face(x, y, z, 0.0f, heights[0]);
      matrix.rotateY180();
      add_front_face(x, y, z, 0.0f, heights[nz]);
      matrix.rotateY180();

      float zsize = (z*2.0f)/nz;

      for (unsigned i = 0; i != nz; ++i) {
        unsigned short cur_vertex = (unsigned short)vertices.size();
        float yOffset0 = heights[i];
        float yOffset1 = heights[i+1];

        add_vertex(vec4(-x, -y + yOffset0, z-(i+0)*zsize, 1), vec4(-1, 0, 0, 0), 0, 0);
        add_vertex(vec4(-x,  y + yOffset0, z-(i+0)*zsize, 1), vec4(-1, 0, 0, 0), 0, 1);
        add_vertex(vec4(-x,  y + yOffset1, z-(i+1)*zsize, 1), vec4(-1, 0, 0, 0), 1, 1);
        add_vertex(vec4(-x, -y + yOffset1, z-(i+1)*zsize, 1), vec4(-1, 0, 0, 0), 1, 0);

        add_vertex(vec4(x, -y + yOffset0, z-(i+0)*zsize, 1), vec4(1, 0, 0, 0), 0, 0);
        add_vertex(vec4(x, -y + yOffset1, z-(i+1)*zsize, 1), vec4(1, 0, 0, 0), 1, 0);
        add_vertex(vec4(x,  y + yOffset1, z-(i+1)*zsize, 1), vec4(1, 0, 0, 0), 1, 1);
        add_vertex(vec4(x,  y + yOffset0, z-(i+0)*zsize, 1), vec4(1, 0, 0, 0), 0, 1);
        
        add_vertex(vec4(-x,  y + yOffset0, z-(i+0)*zsize, 1), vec4(0, 1, 0, 0), 0, 0);
        add_vertex(vec4( x,  y + yOffset0, z-(i+0)*zsize, 1), vec4(0, 1, 0, 0), 0, 1);
        add_vertex(vec4( x,  y + yOffset1, z-(i+1)*zsize, 1), vec4(0, 1, 0, 0), 1, 1);
        add_vertex(vec4(-x,  y + yOffset1, z-(i+1)*zsize, 1), vec4(0, 1, 0, 0), 1, 0);
        
        add_vertex(vec4(-x,  -y + yOffset0, z-(i+0)*zsize, 1), vec4(0, -1, 0, 0), 0, 0);
        add_vertex(vec4(-x,  -y + yOffset1, z-(i+1)*zsize, 1), vec4(0, -1, 0, 0), 1, 0);
        add_vertex(vec4( x,  -y + yOffset1, z-(i+1)*zsize, 1), vec4(0, -1, 0, 0), 1, 1);
        add_vertex(vec4( x,  -y + yOffset0, z-(i+0)*zsize, 1), vec4(0, -1, 0, 0), 0, 1);

        indices.push_back(cur_vertex+0);
        indices.push_back(cur_vertex+1);
        indices.push_back(cur_vertex+2);
        indices.push_back(cur_vertex+0);
        indices.push_back(cur_vertex+2);
        indices.push_back(cur_vertex+3);

        indices.push_back(cur_vertex+4);
        indices.push_back(cur_vertex+5);
        indices.push_back(cur_vertex+6);
        indices.push_back(cur_vertex+4);
        indices.push_back(cur_vertex+6);
        indices.push_back(cur_vertex+7);

        indices.push_back(cur_vertex+8);
        indices.push_back(cur_vertex+9);
        indices.push_back(cur_vertex+10);
        indices.push_back(cur_vertex+8);
        indices.push_back(cur_vertex+10);
        indices.push_back(cur_vertex+11);

        indices.push_back(cur_vertex+12);
        indices.push_back(cur_vertex+13);
        indices.push_back(cur_vertex+14);
        indices.push_back(cur_vertex+12);
        indices.push_back(cur_vertex+14);
        indices.push_back(cur_vertex+15);
      }
    }

    void add_plane(float size, unsigned nx, unsigned ny) {
      add_plane(size, size, nx, ny);
    }

    // add a subdivided size*size plane with nx*ny squares
    void add_plane(float xSize, float ySize, unsigned nx, unsigned ny) {
      float xsize = xSize / nx;
      float ysize = ySize / ny;
      float xSizeBy2 = xSize * 0.5f;
      float ySizeBy2 = ySize * 0.5f;
      for (unsigned i = 0; i != nx; ++i) {
        for (unsigned j = 0; j != ny; ++j) {
          unsigned short cur_vertex = (unsigned short)vertices.size();
          add_vertex(vec4( i*xsize-xSizeBy2, j*ysize-ySizeBy2, 0, 1), vec4(0, 0, 1, 0), 0, 0);
          add_vertex(vec4( i*xsize-xSizeBy2, (j+1)*ysize-ySizeBy2, 0, 1), vec4(0, 0, 1, 0), 0, 1);
          add_vertex(vec4( (i+1)*xsize-xSizeBy2, (j+1)*ysize-ySizeBy2, 0, 1), vec4(0, 0, 1, 0), 1, 1);
          add_vertex(vec4( (i+1)*xsize-xSizeBy2, j*ysize-ySizeBy2, 0, 1), vec4(0, 0, 1, 0), 1, 0);
          indices.push_back(cur_vertex+0);
          indices.push_back(cur_vertex+1);
          indices.push_back(cur_vertex+2);
          indices.push_back(cur_vertex+0);
          indices.push_back(cur_vertex+2);
          indices.push_back(cur_vertex+3);
        }
      }
    }

    

    // add a subdivided size*size plane with nx*ny squares, z value is
    // given by an image
    void add_plane_heightmap(float xSize, float ySize, unsigned nx, unsigned ny, vec4 *normalmap, unsigned hmx, unsigned hmy, float *heightmap) {
      float xsize = xSize / nx;
      float ysize = ySize / ny;
      float xSizeBy2 = xSize * 0.5f;
      float ySizeBy2 = ySize * 0.5f;

      unsigned short cur_vertex = (unsigned short)vertices.size();

      for (unsigned j = 0; j != ny; ++j) {
        for (unsigned i = 0; i != nx; ++i) {
          add_vertex(vec4( i*xsize-xSizeBy2, j*ysize-ySizeBy2, heightmap[hmx*(j+1)+(i+1)], 1), normalmap[nx*j+i], ((float)i)/nx, ((float)j)/ny);
        }
      }

      for (unsigned j = 0; j != ny-1; ++j) {
        for (unsigned i = 0; i != nx-1; ++i) {
          indices.push_back(cur_vertex+ny*(j+0)+(i+0));
          indices.push_back(cur_vertex+ny*(j+0)+(i+1));
          indices.push_back(cur_vertex+ny*(j+1)+(i+1));
          indices.push_back(cur_vertex+ny*(j+0)+(i+0));
          indices.push_back(cur_vertex+ny*(j+1)+(i+1));
          indices.push_back(cur_vertex+ny*(j+1)+(i+0));
        }
      }
    }

  void add_extrude_polygon(vec4 *vertices, int height){

    vec4 nv0 = vec4(vertices[0].x(), vertices[0].y() , vertices[0].z(), vertices[0].w());
    vec4 nv1 = vec4(vertices[1].x(), vertices[1].y() , vertices[1].z(), vertices[0].w());
    vec4 nv2 = vec4(vertices[2].x(), vertices[2].y() , vertices[2].z(), vertices[0].w());
    vec4 nv3 = vec4(vertices[3].x(), vertices[3].y() , vertices[3].z(), vertices[0].w());

    vec4 mv0 = vec4(nv0.x(), nv0.y()+height , nv0.z(), nv0.w());
    vec4 mv1 = vec4(nv1.x(), nv1.y()+height , nv1.z(), nv1.w());
    vec4 mv2 = vec4(nv2.x(), nv2.y()+height , nv2.z(), nv2.w());
    vec4 mv3 = vec4(nv3.x(), nv3.y()+height , nv3.z(), nv3.w());


    add_face(nv0, nv1, nv2, nv3, vec4(0, -1, 0, 0));
    add_face(mv0, mv1, mv2, mv3, vec4(0, 1, 0, 0));
    add_face(nv0, nv1, mv1, mv0, vec4(1, 0, 0, 0));
    add_face(nv1, nv2, mv2, mv1, vec4(1, 0, 0, 0));
    add_face(nv2, nv3, mv3, mv2, vec4(1, 0, 0, 0));
    add_face(nv3, nv0, mv0, mv3, vec4(1, 0, 0, 0));
   
  }


  // add a sphere to the model at the current matrix location
    // as in glutSolidSphere.
    // This is not a very nice sphere so later I must add a geosphere.
    void add_sphere(float radius, unsigned slices, unsigned stacks, float uvscale=1) {
      add_cone_or_sphere(radius, radius*2.0f, slices, stacks, uvscale, true);
    }

    // add a cone to the model at the current matrix location
    // as in glutSolidCone.
    void add_cone(float radius, float height, unsigned slices, unsigned stacks, float uvscale=1) {
      add_cone_or_sphere(radius, height, slices, stacks, uvscale, false);
    }

    // get a mesh mesh from the builder either as VBOs or allocated memory.
    void get_mesh(mesh &s);

    void scale(float x, float y, float z) {
      matrix.scale(x, y, z);
    }

    void translate(float x, float y, float z) {
      matrix.translate(x, y, z);
    }

    void rotate(float angle, float x, float y, float z) {
      matrix.rotate(angle, x, y, z);
    }

    void loadIdentity() {
      matrix.loadIdentity();
    }
  };
}

