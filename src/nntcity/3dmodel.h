namespace octet {

  class model{
    mat4t modelToWorld;

    GLuint texture;

    std::vector<mesh*> meshes;  

    collada_builder builder;

    // container for resources
    resources dict;

  public:
    model(std::string name){      
      
      std::string s("assets/citytex/models/");
      s.append(name);
      s.append(".dae");

      builder.load_xml(s.c_str());

      std::vector<std::string> geometries = builder.get_geometries();

      for(int i=0;i!=geometries.size();++i){
        mesh* mesh1 = new mesh();
        meshes.push_back(mesh1);
        builder.get_mesh(*(meshes[i]), geometries[i].c_str(), dict);
      }
    }

    void render(){
      for(int i=0;i!=meshes.size();++i){
        meshes[i]->render();
      }

    }
  };

}
