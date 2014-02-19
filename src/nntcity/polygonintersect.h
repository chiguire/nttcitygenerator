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
    static void intersectGrid(dynarray<float> &polygon, 
                       float centerX, float centerY, 
                       float separationX, float separationY, 
                       int width, int height, 
                       dynarray<float> &resultVertices, dynarray<short> &resultIndices) {
      resultVertices.reset();
      resultIndices.reset();

      IntersectVertex gridOrigin(centerX-separationY*(width/2.0f), centerY-separationY*(height/2.0f));
      IntersectVertex minGrid(centerX+separationX*(width/2+1), centerY+separationY*(height/2+1));
      IntersectVertex maxGrid(centerX-separationY*(width/2+1), centerY-separationY*(height/2+1));
      
      for (int i = 0; i != polygon.size(); i += 2) {
        IntersectVertex a(polygon[i], polygon[i+1]);

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
          float square[] = {
            gridOrigin.x + i*separationX, gridOrigin.y + i*separationY,
            gridOrigin.x + (i+1)*separationX, gridOrigin.y + (i+1)*separationY 
          };
          dynarray<float> polygonIntersect;
          PolygonIntersections::intersectPolygonAABB(polygon, square, polygonIntersect);
          if (polygonIntersect.size() > 0) {
            unsigned short cur_vertex = (unsigned short)resultVertices.size()/2;
            int num_triangles = (polygonIntersect.size()/2)-2;
            for (auto k = polygonIntersect.begin(); k != polygonIntersect.end(); k++) {
              resultVertices.push_back(*k);
            }
            for (int k = 0; k != num_triangles; k++) {
              resultIndices.push_back(cur_vertex+0);
              resultIndices.push_back(cur_vertex+i+1);
              resultIndices.push_back(cur_vertex+i+2);
            }
          }
        }
      }
    }

    static void intersectPolygonAABB(dynarray<float> &polygon, float bounds[], dynarray<float> &result) {
      IntersectVertex boundsMin(1000000.0f, 1000000.0f);
      IntersectVertex boundsMax(-1000000.0f, -1000000.0f);

      for (int i = 0; i != polygon.size(); i += 2) {
        IntersectVertex v(polygon[i], polygon[i+1]);

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
        return;
      }

      // If polygon is completely inside aabb then return polygon as is
      if (boundsMin.x > bounds[0] && boundsMax.x < bounds[2] &&
        boundsMin.y > bounds[1] && boundsMax.y < bounds[3]) {
        for (auto k = polygon.begin(); k != polygon.end(); k++) {
          result.push_back(*k);
        } 
        return;
      }
      
      dynarray<float> intermediate1;
      dynarray<float> intermediate2;
      dynarray<float> &intermediate = intermediate1;

      //Sutherland-Hodgman
      //Clip left of bounds[0]
      for (int i = 0; i != polygon.size(); i += 2) {
        int previousIndex = (polygon.size()+(i-1)*2)%polygon.size();
        IntersectVertex a(polygon[previousIndex], polygon[previousIndex+1]);
        IntersectVertex b(polygon[i], polygon[i+1]);
        float bound = bounds[0];

        if (a.x >= bound && b.x >= bound) {
          intermediate.push_back(b.x);
          intermediate.push_back(b.y);
        }
        if (a.x >= bound && b.x < bound) {
          float slope = (b.y-a.y)/(b.x-a.x);
          float origin = a.y - slope*a.x;
          float y0 = slope*bound+origin;
          intermediate.push_back(bound);
          intermediate.push_back(y0);
        }
        if (a.x < bound && b.x < bound) {
          //Do nothing
        }
        if (a.x < bound && b.x >= bound) {
          float slope = (b.y-a.y)/(b.x-a.x);
          float origin = a.y - slope*a.x;
          float y0 = slope*bound+origin;
          intermediate.push_back(bound);
          intermediate.push_back(y0);
          intermediate.push_back(b.x);
          intermediate.push_back(b.y);
        }
      }

      intermediate = intermediate2;
      intermediate.reset();

      //Clip top of bounds[1]
      for (int i = 0; i != intermediate1.size(); i += 2) {
        int previousIndex = (intermediate1.size()+(i-1)*2)%intermediate1.size();
        IntersectVertex a(intermediate1[previousIndex], intermediate1[previousIndex+1]);
        IntersectVertex b(intermediate1[i], intermediate1[i+1]);
        float bound = bounds[1];

        if (a.y >= bound && b.y >= bound) {
          intermediate.push_back(b.x);
          intermediate.push_back(b.y);
        }
        if (a.y >= bound && b.y < bound) {
          if (abs(b.x - a.x) < CITY_EPSILON) {
            //slope is infinite
            intermediate.push_back(b.x);
            intermediate.push_back(bound);
          } else {
            float slope = (b.y-a.y)/(b.x-a.x);
            float origin = a.y - slope*a.x;
            float x0 = (bound-origin)/slope;
            intermediate.push_back(x0);
            intermediate.push_back(bound);
          }
        }
        if (a.y < bound && b.y < bound) {
          //Do nothing
        }
        if (a.y < bound && b.y >= bound) {
          if (abs(b.x - a.x) < CITY_EPSILON) {
            //slope is infinite
            intermediate.push_back(b.x);
            intermediate.push_back(bound);
          } else {
            float slope = (b.y-a.y)/(b.x-a.x);
            float origin = a.y - slope*a.x;
            float x0 = (bound-origin)/slope;
            intermediate.push_back(x0);
            intermediate.push_back(bound);
          }
          intermediate.push_back(b.x);
          intermediate.push_back(b.y);
        }
      }

      intermediate = intermediate1;
      intermediate.reset();

      //Clip right of bounds[2]
      for (int i = 0; i != intermediate2.size(); i += 2) {
        int previousIndex = (intermediate2.size()+(i-1)*2)%intermediate2.size();
        IntersectVertex a(intermediate2[previousIndex], intermediate2[previousIndex+1]);
        IntersectVertex b(intermediate2[i], intermediate2[i+1]);
        float bound = bounds[2];

        if (a.x <= bound && b.x <= bound) {
          intermediate.push_back(b.x);
          intermediate.push_back(b.y);
        }
        if (a.x <= bound && b.x > bound) {
          float slope = (b.y-a.y)/(b.x-a.x);
          float origin = a.y - slope*a.x;
          float y0 = slope*bound+origin;
          intermediate.push_back(bound);
          intermediate.push_back(y0);
        }
        if (a.x > bound && b.x > bound) {
          //Do nothing
        }
        if (a.x > bound && b.x <= bound) {
          float slope = (b.y-a.y)/(b.x-a.x);
          float origin = a.y - slope*a.x;
          float y0 = slope*bound+origin;
          intermediate.push_back(bound);
          intermediate.push_back(y0);
          intermediate.push_back(b.x);
          intermediate.push_back(b.y);
        }
      }

      intermediate = intermediate2;
      intermediate.reset();

      //Clip bottom of bounds[3]
      for (int i = 0; i != intermediate1.size(); i += 2) {
        int previousIndex = (intermediate1.size()+(i-1)*2)%intermediate1.size();
        IntersectVertex a(intermediate1[previousIndex], intermediate1[previousIndex+1]);
        IntersectVertex b(intermediate1[i], intermediate1[i+1]);
        float bound = bounds[3];
    
        if (a.y <= bound && b.y <= bound) {
          intermediate.push_back(b.x);
          intermediate.push_back(b.y);
        }
        if (a.y <= bound && b.y > bound) {
          if (abs(b.x - a.x) < CITY_EPSILON) {
            //slope is infinite
            intermediate.push_back(b.x);
            intermediate.push_back(bound);
          } else {
            float slope = (b.y-a.y)/(b.x-a.x);
            float origin = a.y - slope*a.x;
            float x0 = (bound-origin)/slope;
            intermediate.push_back(x0);
            intermediate.push_back(bound);
          }
        }
        if (a.y > bound && b.y > bound) {
          //Do nothing
        }
        if (a.y > bound && b.y <= bound) {
          if (abs(b.x - a.x) < CITY_EPSILON) {
            //slope is infinite
            intermediate.push_back(b.x);
            intermediate.push_back(bound);
          } else {
            float slope = (b.y-a.y)/(b.x-a.x);
            float origin = a.y - slope*a.x;
            float x0 = (bound-origin)/slope;
            intermediate.push_back(x0);
            intermediate.push_back(bound);
          }
          intermediate.push_back(b.x);
          intermediate.push_back(b.y);
        }
      }
      
      //Copy intermediate results to final result
      for (auto k = intermediate.begin(); k != intermediate.end(); k++) {
          result.push_back(*k);
      } 
    }
  };  
}