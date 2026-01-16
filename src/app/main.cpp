#include <core/application/application.hpp>
#include "app_layer.hpp"

int main() {
  Core::Application application("Voxel Engine");
  application.addLayer<AppLayer>();
  application.run();

  return 0;
}
