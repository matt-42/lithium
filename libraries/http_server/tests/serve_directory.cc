#include <filesystem>
#include <fstream>
#include <memory>

#include <lithium_http_server.hh>

#include "test.hh"

using namespace li;

int main() {
  namespace fs = std::filesystem;

  fs::path root(fs::temp_directory_path() / "lithium_test_webroot");

  fs::create_directories(root / "subdir");
  auto root_deleter = [](fs::path* root){ fs::remove_all(*root); };
  std::unique_ptr<fs::path, decltype(root_deleter)> tmp_remover(&root, root_deleter);

  {
    std::cout << (root / "subdir" / "hello.txt").string() << std::endl;
    std::ofstream o((root / "subdir" / "hello.txt").string());
    o << "hello world.";
  }

  CHECK_THROW("cannot serve a non existing dir", serve_directory("/xxx"));
  CHECK_THROW("cannot serve a file", serve_directory(root.string() + "/subdir/hello.txt"));

  http_api my_api;

  std::cout << "Server root is " << root.string() << std::endl;
  my_api.add_subapi("/test", serve_directory(root.string()));
  http_serve(my_api, 12157, s::non_blocking);
  //http_serve(my_api, 12157);

  CHECK_EQUAL("serve_file not found if requesting root serve path", http_get("http://localhost:12157/test").status, 404);
  CHECK_EQUAL("serve_file not found if requesting directory even it exists", http_get("http://localhost:12157/test/subdir").status, 404);
  CHECK_EQUAL("serve_file access denied if out of root", http_get("http://localhost:12157/test/..").status, 404);
  CHECK_EQUAL("serve_file not found", http_get("http://localhost:12157/test/subdir/..").status, 404);
  CHECK_EQUAL("serve_file not found", http_get("http://localhost:12157/test/subdir/xxx").status, 404);

  CHECK_EQUAL("serve_file status 200", http_get("http://localhost:12157/test/subdir/hello.txt").status, 200);
  CHECK_EQUAL("serve_file", http_get("http://localhost:12157/test/subdir/hello.txt").body,
              "hello world.");

  CHECK_EQUAL("serve_file with ..", http_get("http://localhost:12157/test/subdir/../subdir/hello.txt").status, 200);
  CHECK_EQUAL("serve_file with ..", http_get("http://localhost:12157/test/subdir/../subdir/hello.txt").body,
              "hello world.");

  // Test with 1MB file.
  {
    std::ofstream o((root / "subdir" / "big_file.txt").string());
    for (int i = 0; i < 1024*1024; i++) 
      o << "xxxxxxxxxx";
  }

  CHECK_EQUAL("10MB file", http_get("http://localhost:12157/test/subdir/../subdir/big_file.txt").status, 200);
  std::cout << http_get("http://localhost:12157/test/subdir/../subdir/big_file.txt").body.size() << std::endl;
  CHECK_EQUAL("10MB file", http_get("http://localhost:12157/test/subdir/../subdir/big_file.txt").body.size(), 10*1024 * 1024);

}
