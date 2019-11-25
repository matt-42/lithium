#include <iostream>
#include <iod/http_client/http_client.hh>

int main()
{
  iod::http_client::http_client client;
  
  std::cout << client("http://www.google.com").body << std::endl;
}