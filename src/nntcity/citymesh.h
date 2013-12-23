
#include <vector>

namespace octet {

	class CityMesh {
		mesh c_mesh;
		material *mat;

	public:
		CityMesh() {}

		//
		// LORENZO:
		// takes the streetsList and createse a unique mesh
		// as a collection of parallelepipeds
		//
		// still have to work on the rendering part 
		// and on the material rendering
		//
		void init(std::vector<StreetSides> *streetsList) {
			int num_vertices = streetsList->size()*2;

			mesh_builder b;

			b.init(num_vertices*4, num_vertices*6);
			
			for (int i=0; i<streetsList->size(); i++) {
				vec4 v1 = streetsList->at(i).points[0];
				vec4 v2 = streetsList->at(i).points[1];

				float points_distance = v1.z() - v2.z();
				b.scale( 1.0f, 1.0f, points_distance);
				
				b.translate( 0.0f, 0.0f, points_distance);
				b.add_cube(1.0f);
			}

			c_mesh.init();
			b.get_mesh(c_mesh);

		//	mat->make_color(vec4( 1.0f, 0.0f, 0.0f, 1.0f), false, false);
		}

		// just a try on how to draw to boxes as the same mesh
		void debug_createSimpleMesh() {
			mesh_builder b;
			b.init(6*4, 4*6);
			b.scale(1.0f, 1.0f, 5.0f);
			b.add_cube(1.0f);
			b.translate(5.0f, 0.0f, 0.0f);
			b.add_cube(1.0f);
			c_mesh.init();
			b.get_mesh(c_mesh);
		}

		void debugRender() {
			c_mesh.render();
		}

	};
 
}