#include <boost/filesystem.hpp>
#include <iod/http_client/http_client.hh>
#include <iod/silicon/silicon.hh>
#include "test.hh"

using namespace iod;

int main() {

  namespace fs = boost::filesystem;

  fs::path root = fs::unique_path();
  fs::create_directories(root / "subdir");

  {
    std::ofstream o((root / "subdir" / "hello.txt").string());
    o << "hello world.";
  }

  api<http_request, http_response> my_api;

  my_api.add_subapi("/test", serve_directory(root.string()));
  auto ctx = http_serve(my_api, 12352, s::non_blocking);

  CHECK_EQUAL("serve_file not found", http_get("http://localhost:12352/test/subdir/xxx").status, 404);
  CHECK_EQUAL("serve_file", http_get("http://localhost:12352/test/subdir/hello.txt").body, "hello world.");

  fs::remove_all(root);
}
