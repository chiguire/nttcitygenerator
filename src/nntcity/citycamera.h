////////////////////////////////////////////////////////////////////////////////
//
// (C) Dimitri Alvarez, Bogdan Catana, Lorenzo Ciciani, Ciro Duran, Mert Oyman 2013
//
// Procedural Generation for a City Scene
//

namespace octet {

enum CameraMode {
  CAMERAMODE_FREEFORM,
  CAMERAMODE_WALKTHROUGH
};

enum WalkthroughMode {
  WALKTHROUGHMODE_SELECT,
  WALKTHROUGHMODE_ADVANCE,
  WALKTHROUGHMODE_ADVANCED,
  WALKTHROUGHMODE_ROTATE,
  WALKTHROUGHMODE_ROTATED
};

/* From cocos2d-x
static inline float bezierat( float a, float b, float c, float d, float t )
{
    return (powf(1-t,3) * a + 
            3*t*(powf(1-t,2))*b + 
            3*powf(t,2)*(1-t)*c +
            powf(t,3)*d );
}
typedef struct _ccBezierConfig {
    //! end position of the bezier
    CCPoint endPosition;
    //! Bezier control point 1
    CCPoint controlPoint_1;
    //! Bezier control point 2
    CCPoint controlPoint_2;
} ccBezierConfig;
*/

struct LinearConfig {
  vec4 start;
  vec4 end;
  float t;
  int pointEnd;
  Street *street;

  quat startRotation;
  quat endRotation;

  LinearConfig()
  : start(0.0f)
  , end(0.0f)
  , t(0.0f)
  , pointEnd(0)
  , street(NULL)
  , startRotation(0.0f)
  , endRotation(0.0f)
  { }

  vec4 at() {
    return start + (end-start)*t;
  }

  vec3 atSlerp() {
    quat qTemp(vec4(0.0f));
    quat qResult(vec4(0.0f));
    float dot = startRotation.dot(endRotation);

    if (dot < 0)
    {
      dot = -dot;
      qTemp = -endRotation;
    } else {
      qTemp = endRotation;
    }

    if (dot < 0.95f) {
      float angle = acosf(dot);
      qResult = (startRotation*sinf(angle*(1-t)) + qTemp*sinf(angle*t))/sinf(angle);
    } else {
      qResult = (startRotation*(1-t) + endRotation*t).normalize();
    }

    vec3 result(0.0f);

    float qx = qResult.x();
    float qy = qResult.y();
    float qz = qResult.z();
    float qw = qResult.w();

    result[1] = atan2(2*qy*qw-2*qx*qz, 1 - 2*qy*qy - 2*qz*qz)*180.0f/3.14159265358979323846f;
    result[0] = asin(2*qx*qy + 2*qz*qw)*180.0f/3.14159265358979323846f;
    result[2] = 0.0f;

    return result;
  }
};

class camera_controls {
  vec4 camera_position;
  vec3 camera_rotation;

  random randomizer;

  CameraMode cameraMode;
  WalkthroughMode walkthroughMode;
  
  // Useful information when in CAMERAMODE_WALKTHROUGH
  int streetIndexSelected;
  Street *streetSelected;
  LinearConfig streetLerp;

  City *city;

  vec4 mouseCoordinates; //x,y for start position, z,w for difference between current and start
  vec3 startingCameraRotation;
  bool isDragging;
  float freeformCameraZoom;

  bool isInFreeform() {
    return CAMERAMODE_FREEFORM == cameraMode;
  }

  float getCameraZoomFactor() {
    return camera_position[2]/40.0f;
  }

  void selectRandomStreet() {
    int i = randomizer.get(0, city->streetsList.size());
    Street *st = &city->streetsList[i];

    streetLerp.start = st->points[0];
    streetLerp.end = st->points[1];
    streetLerp.t = 0.0f;
    streetLerp.pointEnd = 1;
    streetLerp.street = st;

    orientCameraToStreet(true);
  }

  void selectNewStreet() {
    dynarray <Street *>streetsIntersecting;

    city->getStreetsIntersectingByExtreme(streetLerp.street, streetLerp.pointEnd, streetsIntersecting);

    int i = randomizer.get(0, streetsIntersecting.size());
    Street *st = streetsIntersecting[i];

    if (all(streetLerp.street->points[streetLerp.pointEnd] == st->points[0])) {
      streetLerp.pointEnd = 1;
    } else {
      streetLerp.pointEnd = 0;
    }
    streetLerp.street = st;
    streetLerp.start = st->points[streetLerp.pointEnd == 0? 1: 0];
    streetLerp.end = st->points[streetLerp.pointEnd];
    streetLerp.t = 0.0f;

    orientCameraToStreet();
  }

  void orientCameraToStreet(bool immediate = false) {
    if (isDragging) return;

    if (immediate) {
      vec4 dir = (streetLerp.end - streetLerp.start).normalize();
      camera_rotation[0] = 0.0f;
      camera_rotation[1] = atan2f(-dir.x(), -dir.z())*180/3.14159265358979323846f;
      camera_rotation[2] = 0.0f;
    } else {
      mat4t rotMatrixStart(1.0f);
      rotMatrixStart.rotate(-camera_position.y(), 0.0f, 1.0f, 0.0f);
      rotMatrixStart.rotate(camera_position.x(), 1.0f, 0.0f, 0.0f);

      vec4 dir = (streetLerp.end - streetLerp.start).normalize();
      mat4t rotMatrixEnd(1.0f);
      rotMatrixEnd.rotate(atan2f(-dir.x(), -dir.z())*180.0f/3.14159265358979323846f, 0.0f, 1.0f, 0.0f);
      rotMatrixEnd.rotate(0.0f, 1.0f, 0.0f, 0.0f);

      streetLerp.startRotation = rotMatrixStart.toQuaternion();
      streetLerp.endRotation = rotMatrixEnd.toQuaternion();
      streetLerp.t = 0.0f;
    }
  }

public:
  
  camera_controls()
  : camera_position(0.0f, 0.0f, 10.0f, 0.0f)
  , camera_rotation(45.0f, 0.0f, 0.0f)
  , randomizer(time(NULL))
  , cameraMode(CAMERAMODE_FREEFORM)
  , walkthroughMode(WALKTHROUGHMODE_SELECT)
  , streetIndexSelected(-1)
  , streetSelected(NULL)
  , streetLerp()
  , city(NULL)
  , mouseCoordinates()
  , isDragging(false)
  { }
  
  void init(City *c) {
    city = c;

    if (city && city->streetsList.size() > 0) {
      streetSelected = 0;
      streetLerp.start = vec4(0.0f, 0.0f, 0.0f, 1.0f);
      streetLerp.end = vec4(0.0f, 0.0f, 0.0f, 1.0f);
      streetLerp.t = 1.0f;
    } else {
      printf("ERROR: Initing cameras: Either city is null or streetsList is size 0.\n");
    }
  }

  vec4 &getPosition() { return camera_position; }
  vec3 &getRotation() { return camera_rotation; }
  CameraMode getMode() { return cameraMode; }

  void switchToMode(CameraMode m) {
    cameraMode = m;

    if (cameraMode == CAMERAMODE_WALKTHROUGH) {
      freeformCameraZoom = camera_position[2];
      selectRandomStreet();
      walkthroughMode = WALKTHROUGHMODE_ADVANCE;
    } else {
      camera_position[2] = freeformCameraZoom;
    }
  }

  void switchToNextMode() {
    switchToMode(cameraMode == CAMERAMODE_FREEFORM? CAMERAMODE_WALKTHROUGH: CAMERAMODE_FREEFORM);
  }

  void resetCamera() {
    if (!isInFreeform()) return;
    camera_position = vec4(0.0f, 0.0f, 10.0f, 0.0f);
    camera_rotation = vec3(45.0f, 0.0f, 0.0f);
  }

  void moveWithCamera(const vec4 &direction) {
    if (!isInFreeform()) return;
    mat4t directionMatrix(1.0f);
    directionMatrix.rotate(-camera_rotation[1], 0.0f, 0.0f, 1.0f);
    vec4 dir = direction * getCameraZoomFactor() * directionMatrix;

    camera_position[0] += dir[0];
    camera_position[1] += dir[1];
  }

  void moveCameraUp() {
    if (!isInFreeform()) return;
    camera_position[3] += getCameraZoomFactor();
  }

  void moveCameraDown() {
    if (!isInFreeform()) return;
    camera_position[3] -= getCameraZoomFactor();
  }

  void zoomCameraIn() {
    if (!isInFreeform()) return;
    camera_position[2] -= 0.25f;
    if (camera_position[2] < 0.5f) camera_position[2] = 0.5f;
  }

  void zoomCameraOut() {
    if (!isInFreeform()) return;
    camera_position[2] += 0.25f;
  }

  void rotateCameraLeft() {
    camera_rotation[1] += 5.0f;
    if (camera_rotation[1] >= 360.0f) camera_rotation[1] -= 360.0f;
  }

  void rotateCameraRight() {
    camera_rotation[1] -= 5.0f;
    if (camera_rotation[1] < 0.0f) camera_rotation[1] += 360.0f;
  }

  void rotateCameraUp() {
    camera_rotation[0] -= 5.0f;
    if (camera_rotation[0] < -90.0f) camera_rotation[0] = -90.0f;
  }

  void rotateCameraDown() {
    camera_rotation[0] += 5.0f;
    if (camera_rotation[0] > 90.0f) camera_rotation[0] = 90.0f;
  }

  void startMouseDrag(float mouseX, float mouseY) {
    if (isDragging) return;
    mouseCoordinates[0] = mouseX;
    mouseCoordinates[1] = mouseY;
    mouseCoordinates[2] = 0.0f;
    mouseCoordinates[3] = 0.0f;
    startingCameraRotation = camera_rotation;
    isDragging = true;
  }

  void moveMouseDrag(float mouseX, float mouseY) {
    mouseCoordinates[2] = mouseX - mouseCoordinates[0];
    mouseCoordinates[3] = mouseY - mouseCoordinates[1];

    camera_rotation[1] = startingCameraRotation[1] + mouseCoordinates[2];
    if (camera_rotation[1] < 0.0f) {
      camera_rotation[1] += 360.0f;
    } else if (camera_rotation[1] >= 360.0f) {
      camera_rotation[1] -= 360.0f;
    }

    camera_rotation[0] = startingCameraRotation[0] + mouseCoordinates[3];

    if (camera_rotation[0] > 90.0f) {
      camera_rotation[0] = 90.0f;
    } else if (camera_rotation[0] < -90.0f) {
      camera_rotation[0] = -90.0f;
    }
  }

  void endMouseDrag() {
    isDragging = false;
  }

  void updateCamera() {
    if (isInFreeform()) return;

    if (walkthroughMode == WALKTHROUGHMODE_SELECT) {
      //selectNewStreet();
      streetLerp.t = 0.0f;
      walkthroughMode = WALKTHROUGHMODE_ADVANCE;
    } else if (walkthroughMode == WALKTHROUGHMODE_ADVANCE) {
      vec4 interpolatedPoint = streetLerp.at();
      camera_position[0] = interpolatedPoint.x();
      camera_position[1] = interpolatedPoint.z();
      camera_position[2] = 0.0f;
      camera_position[3] = 3*WATER_LEVEL;
      streetLerp.t += 1.0f/(60.0f*1);
      if (streetLerp.t >= 1.0f) {
        walkthroughMode = WALKTHROUGHMODE_ADVANCED;
      }
    } else if (walkthroughMode == WALKTHROUGHMODE_ADVANCED) {
      selectNewStreet();
      walkthroughMode = WALKTHROUGHMODE_ROTATE;
    } else if (walkthroughMode == WALKTHROUGHMODE_ROTATE) {
      vec3 interpolatedRotation = streetLerp.atSlerp();

      camera_rotation[0] = interpolatedRotation.x();
      camera_rotation[1] = interpolatedRotation.y();

      streetLerp.t += 1.0f/(30.0f);

      if (streetLerp.t >= 1.0f) {
        walkthroughMode = WALKTHROUGHMODE_ROTATED;
      }
    } else if (walkthroughMode == WALKTHROUGHMODE_ROTATED) {
      walkthroughMode = WALKTHROUGHMODE_SELECT;
    }
  }
};
}