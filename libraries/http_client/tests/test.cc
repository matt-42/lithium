#include <iostream>
#include <iod/http_client/http_client.hh>

int main()
{
  iod::http_client client;
  
  std::cout << client.get("http://www.google.com").body << std::endl;
}