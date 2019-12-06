#include <iostream>
#include <li/http_client/http_client.hh>

int main() {
  li::http_client client;

  auto res = client.get("http://www.google.com", s::fetch_headers);
  for (auto pair : res.headers) {
    std::cout << pair.first << ":::" << pair.second << std::endl;
  }
  // std::cout << client.get("http://www.google.com", s::fetch_headers).body << std::endl;
}