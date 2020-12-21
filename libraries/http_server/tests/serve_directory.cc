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

  CHECK_THROW("cannot serve a non existing dir", serve_directory("/xxx"));
  CHECK_THROW("cannot serve a file", serve_directory(root.string() + "/subdir/hello.txt"));

  http_api my_api;

  my_api.add_subapi("/test", serve_directory(root.string()));
  http_serve(my_api, 12347, s::non_blocking);
  //http_serve(my_api, 12347);

  CHECK_EQUAL("serve_file not found if requesting root serve path", http_get("http://localhost:12347/test").status, 404);
  CHECK_EQUAL("serve_file not found if requesting directory even it exists", http_get("http://localhost:12347/test/subdir").status, 404);
  CHECK_EQUAL("serve_file access denied if out of root", http_get("http://localhost:12347/test/..").status, 404);
  CHECK_EQUAL("serve_file not found", http_get("http://localhost:12347/test/subdir/..").status, 404);
  CHECK_EQUAL("serve_file not found", http_get("http://localhost:12347/test/subdir/xxx").status, 404);
  CHECK_EQUAL("serve_file", http_get("http://localhost:12347/test/subdir/hello.txt").body,
              "hello world.");
  CHECK_EQUAL("serve_file with ..", http_get("http://localhost:12347/test/subdir/../subdir/hello.txt").body,
              "hello world.");
}
