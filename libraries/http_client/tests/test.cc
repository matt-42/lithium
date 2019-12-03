#include <iostream>
#include <li/http_client/http_client.hh>

int main()
{
  li::http_client client;
  
  std::cout << client.get("http://www.google.com").body << std::endl;
}