////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Animation example.
//
// Demonstrates a basic scene graph with import from Collada.
//
#include "../../platform/platform.h"

// city headers
#include "../../nntcity/3dmodel.h"
#include "../../nntcity/polygonintersect.h"
#include "../../nntcity/cityobjs.h"
#include "../../nntcity/citymesh.h"
#include "../../nntcity/citycamera.h"

#include "engine.h"

//
// a C++ program starts here with the command line arguments in argv[]
// argc is the number of arguments with argv[0] set to the command name.
//
int main(int argc, char **argv) {
  octet::app_utils::prefix("../../");
  octet::app::init_all(argc, argv);
  octet::engine app(argc, argv);
  app.init();
  octet::app::run_all_apps();
}

