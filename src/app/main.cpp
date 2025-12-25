#include <core/application.hpp>
#include "app_layer.hpp"

int main() {
  Core::Application application("Voxel Engine");
  application.AddLayer<AppLayer>();
  application.Run();

  return 0;
}
