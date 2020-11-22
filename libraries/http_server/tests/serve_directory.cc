#include <filesystem>
#include <fstream>
#include <memory>

#include <lithium_http_server.hh>

#include "test.hh"

using namespace li;

int main() {

  namespace fs = std::filesystem;

  char root_tmp[] = "/tmp/webroot_XXXXXX";
  fs::path root(::mkdtemp(root_tmp));
  fs::create_directories(root / "subdir");
  std::unique_ptr<char, void (*)(char*)> tmp_remover(root_tmp, [](char* tfp){ fs::remove_all(tfp); });

  {
    std::cout << (root / "subdir" / "hello.txt").string() << std::endl;
    std::ofstream o((root / "subdir" / "hello.txt").string());
    o << "hello world.";
  }

  http_api my_api;

  my_api.add_subapi("/test", serve_directory(root.string()));
  http_serve(my_api, 12352, s::non_blocking);
  //http_serve(my_api, 12352);

  CHECK_EQUAL("serve_file not found (CRASH!)", http_get("http://localhost:12352/test/subdir").status, 404);
  CHECK_EQUAL("serve_file not found", http_get("http://localhost:12352/test/subdir/xxx").status,
              404);
  CHECK_EQUAL("serve_file", http_get("http://localhost:12352/test/subdir/hello.txt").body,
              "hello world.");
}
