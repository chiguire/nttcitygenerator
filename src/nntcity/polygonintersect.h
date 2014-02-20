namespace octet {
  const float CITY_EPSILON = std::numeric_limits<float>::epsilon();

  struct IntersectVertex {
    float x;
    float y;
    int i;
    int j;
    IntersectVertex(float _x = 0.0f, float _y = 0.0f, int _i = 0, int _j = 0)
    : x(_x), y(_y), i(_i), j(_j) { }
    IntersectVertex(IntersectVertex &iv)
    : x(iv.x), y(iv.y), i(iv.i), j(iv.j) { }
  };

  class PolygonIntersections {
    
    public:

    // Obtain a series of triangles that represent the intersection of
    // a polygon with a axis-aligned grid, in 2D.
    // polygon must have an even number of floats
    static void intersectGrid(dynarray<vec4> &polygon, 
                       float centerX, float centerY, 
                       float separationX, float separationY, 
                       int width, int height, 
                       dynarray<vec4> &resultVertices, dynarray<unsigned short> &resultIndices) {
      resultVertices.reset();
      resultIndices.reset();

      IntersectVertex gridOrigin(centerX-separationX*(width/2.0f), centerY-separationY*(height/2.0f));
      IntersectVertex minGrid(centerX+separationX*(width/2.0f+1), centerY+separationY*(height/2.0f+1));
      IntersectVertex maxGrid(centerX-separationX*(width/2.0f+1), centerY-separationY*(height/2.0f+1));
      
      for (int i = 0; i != polygon.size(); i ++) {
        IntersectVertex a(polygon[i].x(), polygon[i].z());

        if (a.x < minGrid.x) {
          minGrid.x = a.x;
        }
        if (a.x > maxGrid.x) {
          maxGrid.x = a.x;
        }
        if (a.y < minGrid.y) {
          minGrid.y = a.y;
        }
        if (a.y > maxGrid.y) {
          maxGrid.y = a.y;
        }
      }

      minGrid.i = (int)floor((minGrid.x - gridOrigin.x)/separationX);
      minGrid.j = (int)floor((minGrid.y - gridOrigin.y)/separationY);
      maxGrid.i = (int)floor((maxGrid.x - gridOrigin.x)/separationX);
      maxGrid.j = (int)floor((maxGrid.y - gridOrigin.y)/separationY);

      for (int j = minGrid.j; j <= maxGrid.j; j++) {
        for (int i = minGrid.i; i <= maxGrid.i; i++) {
          dynarray<vec4> polygonIntersect;
          float square[] = {
            gridOrigin.x + i*separationX, gridOrigin.y + j*separationY,
            gridOrigin.x + (i+1)*separationX, gridOrigin.y + (j+1)*separationY 
          };
          //printf("Comparing against square: (%.2f, %.2f), (%.2f, %.2f): ", square[0], square[1], square[2], square[3]);
          PolygonIntersections::intersectPolygonAABB(polygon, square, polygonIntersect);
          if (polygonIntersect.size() > 0) {
            unsigned short cur_vertex = (unsigned short)resultVertices.size();
            int num_triangles = polygonIntersect.size()-2;
            if (num_triangles > 0) {
              for (auto k = polygonIntersect.begin(); k != polygonIntersect.end(); k++) {
                resultVertices.push_back(*k);
              }
              for (int k = 0; k != num_triangles; k++) {
                resultIndices.push_back(cur_vertex+0);
                resultIndices.push_back(cur_vertex+k+1);
                resultIndices.push_back(cur_vertex+k+2);
              }
            }
          }
        }
      }
    }

    static void intersectPolygonAABB(dynarray<vec4> &polygon, float bounds[], dynarray<vec4> &result) {
      IntersectVertex boundsMin(1000000.0f, 1000000.0f);
      IntersectVertex boundsMax(-1000000.0f, -1000000.0f);

      for (int i = 0; i != polygon.size(); i++) {
        IntersectVertex v(polygon[i].x(), polygon[i].z());

        if (v.x < boundsMin.x) {
          boundsMin.x = v.x;
        }
        if (v.y < boundsMin.y) {
          boundsMin.y = v.y;
        }
        if (v.x > boundsMax.x) {
          boundsMax.x = v.x;
        }
        if (v.y > boundsMax.y) {
          boundsMax.y = v.y;
        }
      }

      // Four possibilities where aabb does not clip polygon at all
      if (boundsMax.x < bounds[0] || boundsMin.x > bounds[2] ||
        boundsMax.y < bounds[1] || boundsMin.y > bounds[3]) {
        //printf("No polygon generated.\n");
        return;
      }

      // If polygon is completely inside aabb then return polygon as is
      if (boundsMin.x > bounds[0] && boundsMax.x < bounds[2] &&
        boundsMin.y > bounds[1] && boundsMax.y < bounds[3]) {
        result.reset();
        for (auto k = polygon.begin(); k != polygon.end(); k++) {
          result.push_back(*k);
        } 
        //printf("Whole polygon contained.\n");
        return;
      }
      
      dynarray<vec4> intermediate1;
      dynarray<vec4> intermediate2;
      dynarray<vec4> *intermediate = &intermediate1;

      //Sutherland-Hodgman
      //Clip left of bounds[0]
      for (int i = 0; i != polygon.size(); i++) {
        int previousIndex = (polygon.size()+i-1)%polygon.size();
        IntersectVertex a(polygon[previousIndex].x(), polygon[previousIndex].z());
        IntersectVertex b(polygon[i].x(), polygon[i].z());
        float bound = bounds[0];

        if (a.x >= bound && b.x >= bound) {
          intermediate->push_back(vec4(b.x, 0, b.y, 1));
        }
        if (a.x >= bound && b.x < bound) {
          float slope = (b.y-a.y)/(b.x-a.x);
          float origin = a.y - slope*a.x;
          float y0 = slope*bound+origin;
          intermediate->push_back(vec4(bound, 0, y0, 1));
        }
        if (a.x < bound && b.x < bound) {
          //Do nothing
        }
        if (a.x < bound && b.x >= bound) {
          float slope = (b.y-a.y)/(b.x-a.x);
          float origin = a.y - slope*a.x;
          float y0 = slope*bound+origin;
          intermediate->push_back(vec4(bound, 0, y0, 1));
          intermediate->push_back(vec4(b.x, 0, b.y, 1));
        }
      }

      intermediate = &intermediate2;
      intermediate->reset();

      //Clip top of bounds[1]
      for (int i = 0; i != intermediate1.size(); i++) {
        int previousIndex = (intermediate1.size()+i-1)%intermediate1.size();
        IntersectVertex a(intermediate1[previousIndex].x(), intermediate1[previousIndex].z());
        IntersectVertex b(intermediate1[i].x(), intermediate1[i].z());
        float bound = bounds[1];

        if (a.y >= bound && b.y >= bound) {
          intermediate->push_back(vec4(b.x, 0, b.y, 1));
        }
        if (a.y >= bound && b.y < bound) {
          if (abs(b.x - a.x) < CITY_EPSILON) {
            //slope is infinite
            intermediate->push_back(vec4(b.x, 0, bound, 1));
          } else {
            float slope = (b.y-a.y)/(b.x-a.x);
            float origin = a.y - slope*a.x;
            float x0 = (bound-origin)/slope;
            intermediate->push_back(vec4(x0, 0, bound, 1));
          }
        }
        if (a.y < bound && b.y < bound) {
          //Do nothing
        }
        if (a.y < bound && b.y >= bound) {
          if (abs(b.x - a.x) < CITY_EPSILON) {
            //slope is infinite
            intermediate->push_back(vec4(b.x, 0, bound, 1));
          } else {
            float slope = (b.y-a.y)/(b.x-a.x);
            float origin = a.y - slope*a.x;
            float x0 = (bound-origin)/slope;
            intermediate->push_back(vec4(x0, 0, bound, 1));
          }
          intermediate->push_back(vec4(b.x, 0, b.y, 1));
        }
      }

      intermediate = &intermediate1;
      intermediate->reset();

      //Clip right of bounds[2]
      for (int i = 0; i != intermediate2.size(); i++) {
        int previousIndex = (intermediate2.size()+i-1)%intermediate2.size();
        IntersectVertex a(intermediate2[previousIndex].x(), intermediate2[previousIndex].z());
        IntersectVertex b(intermediate2[i].x(), intermediate2[i].z());
        float bound = bounds[2];

        if (a.x <= bound && b.x <= bound) {
          intermediate->push_back(vec4(b.x, 0, b.y, 1));
        }
        if (a.x <= bound && b.x > bound) {
          float slope = (b.y-a.y)/(b.x-a.x);
          float origin = a.y - slope*a.x;
          float y0 = slope*bound+origin;
          intermediate->push_back(vec4(bound, 0, y0, 1));
        }
        if (a.x > bound && b.x > bound) {
          //Do nothing
        }
        if (a.x > bound && b.x <= bound) {
          float slope = (b.y-a.y)/(b.x-a.x);
          float origin = a.y - slope*a.x;
          float y0 = slope*bound+origin;
          intermediate->push_back(vec4(bound, 0, y0, 1));
          intermediate->push_back(vec4(b.x, 0, b.y, 1));
        }
      }

      intermediate = &intermediate2;
      intermediate->reset();

      //Clip bottom of bounds[3]
      for (int i = 0; i != intermediate1.size(); i++) {
        int previousIndex = (intermediate1.size()+i-1)%intermediate1.size();
        IntersectVertex a(intermediate1[previousIndex].x(), intermediate1[previousIndex].z());
        IntersectVertex b(intermediate1[i].x(), intermediate1[i].z());
        float bound = bounds[3];
    
        if (a.y <= bound && b.y <= bound) {
          intermediate->push_back(vec4(b.x, 0, b.y, 1));
        }
        if (a.y <= bound && b.y > bound) {
          if (abs(b.x - a.x) < CITY_EPSILON) {
            //slope is infinite
            intermediate->push_back(vec4(b.x, 0, bound, 1));
          } else {
            float slope = (b.y-a.y)/(b.x-a.x);
            float origin = a.y - slope*a.x;
            float x0 = (bound-origin)/slope;
            intermediate->push_back(vec4(x0, 0, bound, 1));
          }
        }
        if (a.y > bound && b.y > bound) {
          //Do nothing
        }
        if (a.y > bound && b.y <= bound) {
          if (abs(b.x - a.x) < CITY_EPSILON) {
            //slope is infinite
            intermediate->push_back(vec4(b.x, 0, bound, 1));
          } else {
            float slope = (b.y-a.y)/(b.x-a.x);
            float origin = a.y - slope*a.x;
            float x0 = (bound-origin)/slope;
            intermediate->push_back(vec4(x0, 0, bound, 1));
          }
          intermediate->push_back(vec4(b.x, 0, b.y, 1));
        }
      }
      
      //Copy intermediate results to final result
      result.reset();
      //printf("Clipped result: [");
      for (auto k = intermediate->begin(); k != intermediate->end(); k++) {
          result.push_back(*k);
          //printf("(%.2f, %.2f), ", k->x(), k->y());
      } 
      //printf("]\n");
    }
  };  
}