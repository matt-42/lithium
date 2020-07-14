#include <cstdio>
#include "test.hh"
#include <filesystem>
#include <fstream>
#include <lithium_http_backend.hh>


using namespace li;

int main() {

  namespace fs = std::filesystem;

  fs::path root = std::tmpnam(nullptr);
  fs::create_directories(root / "subdir");

  {
    std::cout << (root / "subdir" / "hello.txt").string() << std::endl;
    std::ofstream o((root / "subdir" / "hello.txt").string());
    o << "hello world.";
  }

  http_api my_api;

  my_api.add_subapi("/test", serve_directory(root.string()));
  http_serve(my_api, 12352, s::non_blocking);
  //http_serve(my_api, 12352);

  CHECK_EQUAL("serve_file not found", http_get("http://localhost:12352/test/subdir/xxx").status,
              404);
  CHECK_EQUAL("serve_file", http_get("http://localhost:12352/test/subdir/hello.txt").body,
              "hello world.");

  fs::remove_all(root);
}
