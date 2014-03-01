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
    // Polygons are a series of vec4s, must be aligned to XZ axis.
    // centerX, centerZ - Geometrical center of the grid
    // separationX, separationY - Dimensions of each grid tile
    // halfSizeY - Dimension in Y of the extruded polygon
    // width, height - Dimensions, in tile, of the grid
    // resultVertices, resultIndices - Resulting polygons of the intersection, in vertex/index mode.
    // borderVertices - Resulting vertices around the polygon border. For later extrusion purposes.
    static void intersectGrid(dynarray<vec4> &polygonPositions,
                       dynarray<vec2> &polygonUVCoords,
                       float centerX, float centerZ, 
                       float separationX, float separationZ, float halfSizeY,
                       int width, int height, 
                       dynarray<vec4> &resultVertices, dynarray<unsigned short> &resultIndices, 
                       dynarray<vec4> &resultNormals, dynarray<vec2> &resultUVCoords) {

      IntersectVertex gridOrigin(centerX-separationX*(width/2.0f), centerZ-separationZ*(height/2.0f));
      IntersectVertex minGrid(centerX+separationX*(width/2.0f+1), centerZ+separationZ*(height/2.0f+1));
      IntersectVertex maxGrid(centerX-separationX*(width/2.0f+1), centerZ-separationZ*(height/2.0f+1));

      resultVertices.reset();
      resultIndices.reset();
      resultNormals.reset();
      resultUVCoords.reset();
      
      for (int i = 0; i != polygonPositions.size(); i ++) {
        IntersectVertex a(polygonPositions[i].x(), polygonPositions[i].z());

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
      minGrid.j = (int)floor((minGrid.y - gridOrigin.y)/separationZ);
      maxGrid.i = (int)floor((maxGrid.x - gridOrigin.x)/separationX);
      maxGrid.j = (int)floor((maxGrid.y - gridOrigin.y)/separationZ);

      // Intersecting XZ-aligned polygons
      for (int j = minGrid.j; j <= maxGrid.j; j++) {
        for (int i = minGrid.i; i <= maxGrid.i; i++) {
          dynarray<vec4> polygonIntersect;
          dynarray<vec4> borderIntersect;
          float square[] = {
            gridOrigin.x + i*separationX, gridOrigin.y + j*separationZ,
            gridOrigin.x + (i+1)*separationX, gridOrigin.y + (j+1)*separationZ 
          };
          //printf("Comparing against square: (%.2f, %.2f), (%.2f, %.2f): ", square[0], square[1], square[2], square[3]);
          PolygonIntersections::intersectPolygonAABB(polygonPositions, square, polygonIntersect);
          if (polygonIntersect.size() > 0) {
            unsigned short cur_vertex = (unsigned short)resultVertices.size();
            int num_triangles = polygonIntersect.size()-2;
            if (num_triangles > 0) {
              
              for (auto k = polygonIntersect.begin(); k != polygonIntersect.end(); k++) {
                (*k)[1] = halfSizeY;
                resultVertices.push_back(*k);
                resultNormals.push_back(vec4(0, 0, 0, 0));
                resultUVCoords.push_back(vec2(0, 0));
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

      // Extruding polygons in the Y-axis
      for (int i = 0; i != polygonPositions.size(); i++) {
        dynarray<vec4> borderVertices;
        vec4 *vecA = &polygonPositions[i];
        vec4 *vecB = &polygonPositions[(i+1)%polygonPositions.size()];
        IntersectVertex a(vecA->x(), vecA->z(), (int)floor((vecA->x() - gridOrigin.x)/separationX), (int)floor((vecA->z() - gridOrigin.y)/separationZ));
        IntersectVertex b(vecB->x(), vecB->z(), (int)floor((vecB->x() - gridOrigin.x)/separationX), (int)floor((vecB->z() - gridOrigin.y)/separationZ));

        intersectLineBorderGrid(a, b, gridOrigin, separationX, separationZ, borderVertices);

        //Calculate line normal
        float angleRadians = atan2f(b.y - a.y, b.x - a.x);
        vec4 normal(cos(angleRadians-3.14159265359f/2.0f), 0.0f, sin(angleRadians-3.14159265359f/2.0f), 1.0f);

        /*
        printf("For polygon: [(%.2f, %.2f, %d, %d), (%.2f, %.2f, %d, %d)], resulting division: [\n", a.x, a.y, a.i, a.j, b.x, b.y, b.i, b.j);
        for (auto k = borderVertices.begin(); k != borderVertices.end(); k++) {
          printf("%.2f, %.2f),\n", k->x(), k->z());
        }
        printf("]\n");
        */
        int num_triangles = borderVertices.size()*2-2;

        if (num_triangles > 0) {
          unsigned short cur_vertex = (unsigned short)resultVertices.size();

          for (auto j = borderVertices.begin(); j != borderVertices.end(); j++) {
            (*j)[1] = halfSizeY;
            resultVertices.push_back(*j);
            (*j)[1] = -halfSizeY;
            resultVertices.push_back(*j);
            resultNormals.push_back(normal);
            resultNormals.push_back(normal);
            resultUVCoords.push_back(vec2(0, 0));
            resultUVCoords.push_back(vec2(0, 0));
          }

          for (int k = 0; k != num_triangles/2; k++) {
            resultIndices.push_back(cur_vertex+k*2);
            resultIndices.push_back(cur_vertex+k*2+1);
            resultIndices.push_back(cur_vertex+k*2+2);
            resultIndices.push_back(cur_vertex+k*2+1);
            resultIndices.push_back(cur_vertex+k*2+3);
            resultIndices.push_back(cur_vertex+k*2+2);
          }
        }
      }

      printf("Vertices: %d, UV Coords: %d\n", resultVertices.size(), resultUVCoords.size());
      //Calculate UV Coordinates over all produced vertices
      vec4 origin = polygonPositions[0];
      origin[1] = 0.0f;
      vec4 a = polygonPositions[1] - origin;
      vec4 b = polygonPositions[2] - origin;
      a[1] = 0.0f;
      b[1] = 0.0f;
      float aLen = a.length();
      float bLen = b.length();

      for (int i = 0; i != resultVertices.size(); i++) {
        vec4 pos = resultVertices[i];
        vec2 *uv = &resultUVCoords[i];
        pos[1] = 0.0f;
        pos = pos - origin;

        uv[0] = pos.dot(a)/aLen;
        uv[1] = pos.dot(b)/bLen;
      }
    }

    // Given a polygon, intersect it with an AABB defined by bounds, outputing the
    // vertices in result.
    // bounds is defined as {x0, y0, x1, y1}, given that x0 < x1 && y0 < y1.
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
    
    // Intersects
    static void intersectLineBorderGrid(const IntersectVertex &a, const IntersectVertex &b, const IntersectVertex &gridOrigin,
                                 float separationX, float separationZ, dynarray<vec4> &borderVertices) {

      //printf("Intersection between (%.2f, %.2f) and (%.2f, %.2f)\n", a.x, a.y, b.x, b.y);
      
      borderVertices.push_back(vec4(a.x, 0, a.y, 1.0f));
      
      if (abs(b.x - a.x) < CITY_EPSILON) {
        //slope == 0 (x == b.x)
        float origin = a.x;
        //float x0 = origin;
        //float y0 = any;
        
        //int stepX = not valid;
        int stepY = a.y < b.y? 1: -1;
        //int startX = not valid;
        int startY = a.y < b.y? 1: 0;

        //which position and box of the grid we're in
        vec2 start(a.x, a.y);

        vec2 sideDistYp(origin, gridOrigin.y+(a.j+startY)*separationZ);
        float sideDistY = (sideDistYp - start).length();

        //length of ray from one x or y-side to next x or y-side
        vec2 deltaDistYp(0.0f, stepY*separationZ);
        
        float lineLength = sqrtf((b.y-a.y)*(b.y-a.y)+(b.x-a.x)*(b.x-a.x));
        while (sideDistY < lineLength) {
          borderVertices.push_back(vec4(sideDistYp.x(), 0, sideDistYp.y(), 1.0f)); 
          sideDistYp += deltaDistYp;
          sideDistY = (sideDistYp - start).length();
        }
      } else if (abs(b.y-a.y) < CITY_EPSILON) {
        //slope == 0 (y == b.y)
        float origin = a.y;
        //float x0 = any;
        //float y0 = origin;
        
        int stepX = a.x < b.x? 1: -1;
        //int stepY = not valid;
        int startX = a.x < b.x? 1: 0;
        //int startY = not valid;

        //which position and box of the grid we're in
        vec2 start(a.x, a.y);

        vec2 sideDistXp(gridOrigin.x+(a.i+startX)*separationX, origin);
        float sideDistX = (sideDistXp - start).length();

        //length of ray from one x or y-side to next x or y-side
        vec2 deltaDistXp(stepX*separationX, 0.0f);
        
        float lineLength = sqrtf((b.y-a.y)*(b.y-a.y)+(b.x-a.x)*(b.x-a.x));
        while (sideDistX < lineLength) {
          borderVertices.push_back(vec4(sideDistXp.x(), 0, sideDistXp.y(), 1.0f));
          sideDistXp += deltaDistXp;
          sideDistX = (sideDistXp - start).length();
        }
      } else if (abs(b.x - a.x) < abs(b.y-a.y)) {
        //printf("Y is longer than X\n");
        // Digital Differential Analysis --  http://lodev.org/cgtutor/raycasting.html
        float slope = (b.x-a.x)/(b.y-a.y);
        float origin = a.x - slope*a.y;
        //float x0 = slope*_y0_+origin;
        //float y0 = (_x0_-origin)/slope;
        
        int stepX = a.x < b.x? 1: -1;
        int stepY = a.y < b.y? 1: -1;
        int startX = a.x < b.x? 1: 0;
        int startY = a.y < b.y? 1: 0;

        //which position and box of the grid we're in
        vec2 start(a.x, a.y);

        vec2 sideDistXp(gridOrigin.x+(a.i+startX)*separationX, ((gridOrigin.x+(a.i+startX)*separationX)-origin)/slope);
        vec2 sideDistYp((gridOrigin.y+(a.j+startY)*separationZ)*slope+origin, gridOrigin.y+(a.j+startY)*separationZ);
        float sideDistX = (sideDistXp - start).length();
        float sideDistY = (sideDistYp - start).length();

        //length of ray from one x or y-side to next x or y-side
        vec2 deltaDistXp(stepX*separationX, stepX*separationX/slope);
        vec2 deltaDistYp(stepY*separationZ*slope, stepY*separationZ);
        
        float lineLength = sqrtf((b.y-a.y)*(b.y-a.y)+(b.x-a.x)*(b.x-a.x));
        while (sideDistX < lineLength || sideDistY < lineLength) {
          //printf("Step: [lineLength: (%.2f), sideDistXp: (%.2f, %.2f), sideDistYp: (%.2f, %.2f), sideDistX: %.2f, sideDistY: %.2f, deltaDistXp(%.2f, %.2f), deltaDistYp(%.2f, %.2f) ",
          //  lineLength, sideDistXp.x(), sideDistXp.y(), sideDistYp.x(), sideDistYp.y(), sideDistX, sideDistY, deltaDistXp.x(), deltaDistXp.y(), deltaDistYp.x(), deltaDistYp.y());
          if (sideDistX < sideDistY) {
            borderVertices.push_back(vec4(sideDistXp.x(), 0, sideDistXp.y(), 1.0f));
            sideDistXp += deltaDistXp;
            sideDistX = (sideDistXp - start).length();
          } else {
            borderVertices.push_back(vec4(sideDistYp.x(), 0, sideDistYp.y(), 1.0f)); 
            sideDistYp += deltaDistYp;
            sideDistY = (sideDistYp - start).length();
          }
          //printf("Last point: (%.2f, %.2f)\n", borderVertices[borderVertices.size()-1].x(), borderVertices[borderVertices.size()-1].z());
        }
      } else {
        //printf("X is longer than Y\n");
        // Digital Differential Analysis --  http://lodev.org/cgtutor/raycasting.html
        float slope = (b.y-a.y)/(b.x-a.x);
        float origin = a.y - slope*a.x;
        //float x0 = (_y0_-origin)/slope;
        //float y0 = slope*_x0_+origin;
        
        int stepX = a.x < b.x? 1: -1;
        int stepY = a.y < b.y? 1: -1;
        int startX = a.x < b.x? 1: 0;
        int startY = a.y < b.y? 1: 0;

        //which position and box of the grid we're in
        vec2 start(a.x, a.y);

        vec2 sideDistXp(gridOrigin.x+(a.i+startX)*separationX, (gridOrigin.x+(a.i+startX)*separationX)*slope+origin);
        vec2 sideDistYp(((gridOrigin.y+(a.j+startY)*separationZ)-origin)/slope, gridOrigin.y+(a.j+startY)*separationZ);
        float sideDistX = (sideDistXp - start).length();
        float sideDistY = (sideDistYp - start).length();

        //length of ray from one x or y-side to next x or y-side
        vec2 deltaDistXp(stepX*separationX, stepX*slope*separationX);
        vec2 deltaDistYp(stepY*separationZ/slope, stepY*separationZ);
        
        float lineLength = sqrtf((b.y-a.y)*(b.y-a.y)+(b.x-a.x)*(b.x-a.x));
        while (sideDistX < lineLength || sideDistY < lineLength) {
          //printf("Step: [lineLength: (%.2f), sideDistXp: (%.2f, %.2f), sideDistYp: (%.2f, %.2f), sideDistX: %.2f, sideDistY: %.2f, deltaDistXp(%.2f, %.2f), deltaDistYp(%.2f, %.2f) ",
          //  lineLength, sideDistXp.x(), sideDistXp.y(), sideDistYp.x(), sideDistYp.y(), sideDistX, sideDistY, deltaDistXp.x(), deltaDistXp.y(), deltaDistYp.x(), deltaDistYp.y());
          if (sideDistX < sideDistY) {
            borderVertices.push_back(vec4(sideDistXp.x(), 0, sideDistXp.y(), 1.0f));
            sideDistXp += deltaDistXp;
            sideDistX = (sideDistXp - start).length();
          } else {
            borderVertices.push_back(vec4(sideDistYp.x(), 0, sideDistYp.y(), 1.0f)); 
            sideDistYp += deltaDistYp;
            sideDistY = (sideDistYp - start).length();
          }
          //printf("Last point: (%.2f, %.2f)\n", borderVertices[borderVertices.size()-1].x(), borderVertices[borderVertices.size()-1].z());
        }
      }
      borderVertices.push_back(vec4(b.x, 0, b.y, 1.0f));
    }
  };  
}