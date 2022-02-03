#include <filesystem>
#include <fstream>
#include <memory>

#include <lithium_http_server.hh>

using namespace li;

int main() {
  namespace fs = std::filesystem;

  fs::path root(fs::temp_directory_path() / "lithium_bench_file_serving");

  fs::create_directories(root / "subdir");
  auto root_deleter = [](fs::path* root){ fs::remove_all(*root); };
  std::unique_ptr<fs::path, decltype(root_deleter)> tmp_remover(&root, root_deleter);


  std::string str_10k = "";
  // Test with 1MB file.
  {
    std::ofstream o((root / "subdir" / "file_10K.txt").string());
    for (int i = 0; i < 1024; i++)
      str_10k += "x";

    o << str_10k;
  }

  http_api my_api;

  std::cout << "Server root is " << root.string() << std::endl;
  my_api.add_subapi("/test", serve_directory(root.string()));
  my_api.get("/test_str") = [&] (http_request& request, http_response& response) {
    response.write(str_10k);
  };

  std::string file_path = (root / "subdir" / "file_10K.txt").string();
  my_api.get("/test_static") = [&] (http_request& request, http_response& response) {
    response.write_file(file_path.c_str());
  };
  my_api.get("/test_sendfile") = [&] (http_request& request, http_response& response) {
    response.write_file(file_path.c_str());
  };

  // http_serve(my_api, 12357, s::non_blocking);
  http_serve(my_api, 12357);

}
