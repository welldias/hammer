/* Compiles the cgltf implementation. Kept in its own translation unit,
 * built without hammer's strict warning flags, since this is vendored
 * third-party code (see src/CMakeLists.txt). */
#define CGLTF_IMPLEMENTATION
#include <cgltf.h>
