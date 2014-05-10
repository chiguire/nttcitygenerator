namespace octet {

  class ModelBuilder{

    collada_builder builder;

  public:

    void loadModel(char* modelPath){
      builder.load_xml(modelPath);
    }

    collada_builder* getColladaBuilder(){
      return &builder;
    }
  };


  class Model{
  protected:
    mat4t modelToWorld;

    std::vector<mesh*> meshes;  

    // container for resources
    resources dict;

    // how many lives do we have?
    int ref_count;

    std::string stringMaterial;

  public:
    Model(ModelBuilder* builder){      
    
      std::vector<std::string> geometries = builder->getColladaBuilder()->get_geometries();

      for(int i=0;i!=geometries.size();++i){
        mesh* mesh1 = new mesh();
        meshes.push_back(mesh1);
        builder->getColladaBuilder()->get_mesh(*(meshes[i]), geometries[i].c_str(), dict);
      }

      this->modelToWorld = mat4t(1.0f);

      ref_count = 0;

      this->stringMaterial="";
    }

    Model(const Model& rhs){

      this->modelToWorld = rhs.modelToWorld;

      std::vector<mesh*> m; 

      for(int i=0; i!=rhs.meshes.size();++i){
        mesh* mesh1 = new mesh();
        *mesh1 = *(rhs.meshes[i]);
        m.push_back(mesh1);
      }
      this->meshes = m;
      this->dict = rhs.dict;
      this->ref_count = rhs.ref_count;
    }


    ~Model(){
      for(int i = 0; i != meshes.size(); ++i){
        delete meshes[i];
      }
      //printf("DESTRUCTOR\n");
    }

    void render(){
      for(int i=0;i!=meshes.size();++i){
        meshes[i]->render();
      }
    }

    mat4t getModelToWorld(){
      return this->modelToWorld;
    }

    std::string getMaterial(){
      return this->stringMaterial;
    }


    // give this resource an extra life
    void add_ref() {
      ref_count++;
    }
    // remove a life from this resource and delete it if it is dead.
    void release() {
      if (--ref_count == 0) {
        delete this;
      }
    }
  };

  class LampModel:public Model{
  public:
    LampModel(ModelBuilder* builder,vec4 translation, float rotation):Model(builder){
      //Apply the scale and rotation in every model

      this->modelToWorld.translate(translation.x(),translation.y(),translation.z());
      this->modelToWorld.rotateX(-90.0f);
      this->modelToWorld.rotateZ(rotation);
      this->modelToWorld.scale(0.010f,0.010f,0.010f);
      this->stringMaterial = "Lamp";
      
    }

    LampModel(const LampModel & rhs):Model(rhs){}
  };

  class TrafficLight:public Model{
  public:
    TrafficLight(ModelBuilder* builder,vec4 translation, float rotation):Model(builder){
      //Apply the scale and rotation in every model

      this->modelToWorld.translate(translation.x(),translation.y(),translation.z());
      this->modelToWorld.rotateX(-90.0f);
      this->modelToWorld.rotateZ(rotation);
      this->modelToWorld.scale(0.001f,0.001f,0.001f);
      this->stringMaterial = "Traffic Light";

    }

    TrafficLight(const TrafficLight & rhs):Model(rhs){}
  };

  class Hydrant:public Model{
  public:
    Hydrant(ModelBuilder* builder,vec4 translation, float rotation):Model(builder){
      //Apply the scale and rotation in every model

      this->modelToWorld.translate(translation.x(),translation.y(),translation.z());
      this->modelToWorld.rotateX(-90.0f);
      this->modelToWorld.rotateZ(rotation);
      this->modelToWorld.scale(0.016f,0.016f,0.016f);
      this->stringMaterial = "Hydrant";

    }

    Hydrant(const Hydrant & rhs):Model(rhs){}
  };

  class PostBox:public Model{
  public:
    PostBox(ModelBuilder* builder,vec4 translation, float rotation):Model(builder){
      //Apply the scale and rotation in every model

      this->modelToWorld.translate(translation.x(),translation.y(),translation.z());
      this->modelToWorld.rotateY(rotation);
      this->modelToWorld.scale(0.0015f,0.0015f,0.0015f);
      this->stringMaterial = "Postbox";
    }

    PostBox(const PostBox & rhs):Model(rhs){}
  };

  class Tree:public Model{
  public:
    Tree(ModelBuilder* builder,vec4 translation, float rotation):Model(builder){
      //Apply the scale and rotation in every model

      this->modelToWorld.translate(translation.x(),translation.y(),translation.z());
      this->modelToWorld.rotateX(-90.0f);
      this->modelToWorld.scale(0.08f,0.08f,0.08f);
      this->stringMaterial = "Tree";
    }

    Tree(const Tree & rhs):Model(rhs){}
  };

  class Tree2:public Model{
  public:
    Tree2(ModelBuilder* builder,vec4 translation, float rotation):Model(builder){
      //Apply the scale and rotation in every model

      this->modelToWorld.translate(translation.x(),translation.y(),translation.z());
      this->modelToWorld.rotateX(-90.0f);
      this->modelToWorld.rotateZ(rotation);
      this->modelToWorld.scale(0.003f,0.003f,0.003f);
      this->stringMaterial = "Tree2";
    }

    Tree2(const Tree2 & rhs):Model(rhs){}
  };

  class Bench:public Model{
  public:
    Bench(ModelBuilder* builder,vec4 translation, float rotation):Model(builder){
      //Apply the scale and rotation in every model

      this->modelToWorld.translate(translation.x(),translation.y(),translation.z());
      this->modelToWorld.rotateX(-90.0f);
      this->modelToWorld.rotateZ(90.0f);
      this->modelToWorld.rotateZ(rotation);
      this->modelToWorld.scale(0.001f,0.001f,0.001f);
      this->stringMaterial = "Bench";
    }

    Bench(const Bench & rhs):Model(rhs){}
  };

}
