#include <iostream>
#include <cstdio>
#include "mhdpp.hh"

IOD_SYMBOL(name)

int main()
{
  mhd::mhd_api api;

  api["/test"] = [] (mhd::mhd_ctx& ctx)
    {
      //auto params = ctx.parameters(s::name = std::string());
      //std::cout << params.name << std::endl;
      // ctx.respond(params.name.c_str());
      ctx.respond("hello world\n");
      // ctx.respond("hello worldhello worldhello worldhello worldhello worldhello worldhello worldhello worldhello worldhello worldhello worldhello worldhello worldhello worldhello worldhello worldhello worldhello world\n");
    };

  mhd::mhd_server server(s::linux_epoll = true, s::port = 1212, s::nthreads = 1);
  server.serve(api);

  std::getchar();

  server.stop();
}

