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

  public:
    Model(ModelBuilder* builder){      
    
      std::vector<std::string> geometries = builder->getColladaBuilder()->get_geometries();

      for(int i=0;i!=geometries.size();++i){
        mesh* mesh1 = new mesh();
        meshes.push_back(mesh1);
        builder->getColladaBuilder()->get_mesh(*(meshes[i]), geometries[i].c_str(), dict);
      }

      this->modelToWorld = mat4t(1.0f);
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
    }


    ~Model(){
      for(int i = 0; i != meshes.size(); ++i){
        delete meshes[i];
      }

    }

    void render(){
      for(int i=0;i!=meshes.size();++i){
        meshes[i]->render();
      }
    }

    mat4t getModelToWorld(){
      return this->modelToWorld;
    }
  };

  class LampModel:public Model{
  public:
    LampModel(ModelBuilder* builder,vec4 translation, float rotation):Model(builder){
      //Apply the scale and rotation in every model

      this->modelToWorld.translate(translation.x(),translation.y(),translation.z());
      this->modelToWorld.rotateX(-90.0f);
      this->modelToWorld.rotateY(rotation);
      this->modelToWorld.scale(0.010f,0.010f,0.010f);
      
    }

    LampModel(const LampModel & rhs):Model(rhs){}
  };

}
