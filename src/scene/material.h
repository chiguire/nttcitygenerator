////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Material 
//
//
// Materials are represented as GL textures with solid colours as single pixel textures.
// This simplifies shader design.
//

namespace octet {
  class material : public resource {
    // material
    ref<param> diffuse;
    ref<param> ambient;
    ref<param> emission;
    ref<param> specular;
    ref<param> bump;
    ref<param> shininess;

	ref<param> building_a;
	ref<param> building_b;
	ref<param> building_c; 

    void bind_textures() const {
      // set textures 0, 1, 2, 3 to their respective values
      diffuse->render(0, GL_TEXTURE_2D);
      ambient->render(1, GL_TEXTURE_2D);
      emission->render(2, GL_TEXTURE_2D);
      specular->render(3, GL_TEXTURE_2D);
      bump->render(4, GL_TEXTURE_2D);
      shininess->render(5, GL_TEXTURE_2D);
      glActiveTexture(GL_TEXTURE0);
    }

	void bind_textures_buildings() const {
	  diffuse->render(0, GL_TEXTURE_2D);
      ambient->render(1, GL_TEXTURE_2D);
      emission->render(2, GL_TEXTURE_2D);
      specular->render(3, GL_TEXTURE_2D);
      bump->render(4, GL_TEXTURE_2D);
	  shininess->render(5, GL_TEXTURE_2D);
	  building_a->render(6, GL_TEXTURE_2D);
	  building_b->render(7, GL_TEXTURE_2D);
	  building_c->render(8, GL_TEXTURE_2D);
	  glActiveTexture(GL_TEXTURE0); 
	  
	}

    void init(param *param_) {
      specular = diffuse = ambient = param_;
      emission = new param(vec4(0, 0, 0, 0));
      bump = new param(vec4(0, 0, 0, 0));
      shininess = new param(vec4(30.0f/255, 0, 0, 0));
    }

  public:
    RESOURCE_META(material)

    // default constructor makes a blank material.
    material() {
      diffuse = 0;
      ambient = 0;
      emission = 0;
      specular = 0;
      bump = 0;
      shininess = 0;
    }

    // don't use this too much, it creates a new image every time.
    material(const char *texture, sampler *sampler_ = 0) {
      init(new param(new image(texture), sampler_));
    }

    // don't use this too much, it creates a new image every time.
    material(const vec4 &color) {
      init(new param(color));
    }

    material(image *img, bool shiny=true, float shininess_=30.0f/255) {
      //init(new param(img));
      param *p = new param(img);
      init(p, p, new param(vec4(0.0f)), shiny? p:new param(vec4(0.0f)), new param(vec4(0.0f)), new param(vec4(shininess_, 0.0f, 0.0f, 0.0f)));
    }

	material( image *img_a, image *img_b, image *img_c, image *img_d, bool shiny=true, float shininess_=30.0f/255) {
		param *pa = new param(img_a);
		param *pb = new param(img_b);
		param *pc = new param(img_c);
		param *pd = new param(img_d); 
		init_building(pa, pa, new param(vec4(0.0f)), shiny? pa:new param(vec4(0.0f)), new param(vec4(0.0f)), new param(vec4(shininess_, 0.0f, 0.0f, 0.0f)), pb, pc, pd); 
	}

    material(image *img, image *bumpImg, bool shiny=true, float shininess_=30.0f/255) {
      param *p = new param(img);
      emission = new param(vec4(0));
      specular = shiny? new param(vec4(1, 1, 1, 0)): new param(vec4(0));
      bump = new param(bumpImg);
      shininess = new param(vec4(shininess_, 0.0f, 0.0f, 0.0f));
      init(p, p, emission, specular, bump, shininess);
    }
/*
    material(image *img, const vec4 &color, bool bumpy, bool shiny) {
      img->multiplyColor(color);
      param *p = new param(img);
      emission = new param(vec4(0, 0, 0, 0));
      specular = shiny ? new param(vec4(1, 1, 1, 0)) : new param(vec4(0, 0, 0, 0));
      bump = bumpy ? new param(new image("!bump")) : new param(vec4(0.5f, 0.5f, 1, 0));
      shininess = new param(vec4(30.0f/255, 0, 0, 0));
      init(p, p, emission, specular, bump, shininess);
    }
*/
    void visit(visitor &v) {
      v.visit(diffuse, atom_diffuse);
      v.visit(ambient, atom_ambient);
      v.visit(emission, atom_emission);
      v.visit(specular, atom_specular);
      v.visit(bump, atom_bump);
      v.visit(shininess, atom_shininess);
    }

    void init(param *diffuse, param *ambient, param *emission, param *specular, param *bump, param *shininess) {
      this->diffuse = diffuse;
      this->ambient = ambient;
      this->emission = emission;
      this->specular = specular;
      this->bump = bump;
      this->shininess = shininess;
    }

	void init_building(param *diffuse, param *ambient, param *emission, param *specular, param *bump, param *shininess, param *building_a, param *building_b, param *building_c) {
      this->diffuse = diffuse;
      this->ambient = ambient;
      this->emission = emission;
      this->specular = specular;
      this->bump = bump;
      this->shininess = shininess;
	  this->building_a = building_a;
	  this->building_b = building_b;
	  this->building_c = building_c;

    }

    // make a solid color with a specular highlight
    void make_color(const vec4 &color, bool bumpy, bool shiny) {
      diffuse = ambient = new param(color);
      emission = new param(vec4(0, 0, 0, 0));
      specular = shiny ? new param(vec4(1, 1, 1, 0)) : new param(vec4(0, 0, 0, 0));
      bump = bumpy ? new param(new image("!bump")) : new param(vec4(0.5f, 0.5f, 1, 0));
      shininess = new param(vec4(30.0f/255, 0, 0, 0));
    }

    void render(bump_shader &shader, const mat4t &modelToProjection, const mat4t &modelToCamera, vec4 *light_uniforms, int num_light_uniforms, int num_lights) const {
      shader.render(modelToProjection, modelToCamera, light_uniforms, num_light_uniforms, num_lights);
      bind_textures();
    }

	void renderBuilding(city_buildings_bump_shader &shader, const mat4t &modelToProjection, const mat4t &modelToCamera, vec4 *light_uniforms, int num_light_uniforms, int num_lights, float building_height, float building_area) const {
      shader.render(modelToProjection, modelToCamera, light_uniforms, num_light_uniforms, num_lights, building_height, building_area);
      bind_textures_buildings();
    }

    void render_skinned(bump_shader &shader, const mat4t &cameraToProjection, const mat4t *modelToCamera, int num_nodes, vec4 *light_uniforms, int num_light_uniforms, int num_lights) const {
      shader.render_skinned(cameraToProjection, modelToCamera, num_nodes, light_uniforms, num_light_uniforms, num_lights);
      bind_textures();
    }

	///////////////////////////////////////////////////
	void render_road(city_bump_shader &shader, const mat4t &modelToProjection, const mat4t &modelToCamera, vec4 *light_uniforms, int num_light_uniforms, int num_lights) const {
      shader.render(modelToProjection, modelToCamera, light_uniforms, num_light_uniforms, num_lights);
      bind_textures();
    }

  };
}

