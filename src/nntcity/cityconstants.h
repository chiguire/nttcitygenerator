namespace octet {

  class CityConstants {
   
  public:
    //City
    static const float STREET_WIDTH;
    static const float ROAD_WIDTH;
    static const float PAVEMENT_WIDTH;
    static const float ROAD_HEIGHT;
    static const float PAVEMENT_HEIGHT;
    static const float LAMPS_SEPARATION;

    //CityMesh
    static const float HEIGHT_FACTOR;
    static const float WATER_LEVEL;
    static const float BRIDGE_LEVEL;
    static const float MULTIPLIER;
    static const float OFFSET_X;
    static const float OFFSET_Y;
    static const float ROAD_RAISE;
    static const float PAVEMENT_RAISE;
	  static const float BUILDING_ROOF_HEIGHT;
	  static const float BUILDING_BASEMENT_HEIGHT;

    
  };

  const float CityConstants::STREET_WIDTH = 0.98f;
  const float CityConstants::PAVEMENT_WIDTH = 0.25f;
  const float CityConstants::ROAD_WIDTH = CityConstants::STREET_WIDTH - 2*CityConstants::PAVEMENT_WIDTH;
  const float CityConstants::ROAD_HEIGHT = 0.04f;
  const float CityConstants::PAVEMENT_HEIGHT = 0.042f;
  const float CityConstants::LAMPS_SEPARATION = 2.0f;
  
  const float CityConstants::HEIGHT_FACTOR = 1.0f/255.0f;
  const float CityConstants::WATER_LEVEL = 0.3f;
  const float CityConstants::BRIDGE_LEVEL = 0.35f;
  const float CityConstants::MULTIPLIER = 0.5f;
  const float CityConstants::OFFSET_X = 0.25f;
  const float CityConstants::OFFSET_Y = 0.25f;
  const float CityConstants::ROAD_RAISE = CityConstants::ROAD_HEIGHT;
  const float CityConstants::PAVEMENT_RAISE = CityConstants::PAVEMENT_HEIGHT;
  const float CityConstants::BUILDING_ROOF_HEIGHT = 0.05f;
  const float CityConstants::BUILDING_BASEMENT_HEIGHT = 1.0f;
}