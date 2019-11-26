#pragma once

#include <boost/context/continuation.hpp>
#include <string_view>
#include <cstring>
#include <microhttpd.h>
#include <functional>
#include <fstream>
#include <sstream>
#include <map>
#include "iod/metamap/metamap.hh"

namespace ctx=boost::context;

IOD_SYMBOL(one_thread_per_connection);
IOD_SYMBOL(linux_epoll)
IOD_SYMBOL(nthreads)
IOD_SYMBOL(https_key_path)
IOD_SYMBOL(https_cert_path)
IOD_SYMBOL(select)
IOD_SYMBOL(port)

namespace mhd
{
  using namespace iod;
  namespace impl {
    int
    mhd_handler(void * cls,
                struct MHD_Connection * connection,
                const char * url,
                const char * method,
                const char * version,
                const char * upload_data,
                size_t * upload_data_size,
                void ** ptr);
  }
  struct mhd_ctx
  {
    template <typename... T> auto parameters(T&&... ps)
    {
      auto params = make_metamap(ps...);

      params.name = "0";
      while (!end_of_body_)
      {
        // std::cout << "end_of_body_1 " << end_of_body_ << std::endl;        
        continuation_sink_ = continuation_sink_.resume();
        if (post_key_)
        {
          // If the main handler got a new post parameter.
          // decode it here.
          params.name = post_data_;
          post_key_ = nullptr;
        }
        // std::cout << "end_of_body_2 " << end_of_body_ << std::endl;        
      }

      return params;
    }
    
    // template <typename... T> auto post_parameters(T&&...);
    // template <typename... T> auto get_parameters(T&&...);
    // template <typename... T> auto read_large_post_parameter(char* key, void(*data, datasize) callback);

    // template <typename S> auto header(S key);
    // template <typename S> auto set_cookie(S key);

    // inline std::string_view write_body(FILE* output_file);
    // template <typename... T> auto respond(T...);

    auto respond(const char* str) {
      response_ = MHD_create_response_from_buffer(strlen(str),
                                                 (void*) str,
                                                 MHD_RESPMEM_MUST_COPY);
    }
    
    friend
    int impl::mhd_handler(void * cls,
                          struct MHD_Connection * connection,
                          const char * url,
                          const char * method,
                          const char * version,
                          const char * upload_data,
                          size_t * upload_data_size,
                          void ** ptr);

    // private:
    // Request data.
    MHD_Connection* connection_ = nullptr;
    const char* url_;
    const char* method_;
    const char* http_version_;

    // Response data.
    MHD_Response* response_ = nullptr;
    int response_status_ = 200;

    bool done_ = false;

    MHD_PostProcessor* post_processor_ = nullptr;
    const char *post_key_ = nullptr;
    const char *post_filename_ = nullptr;
    const char *post_content_type_ = nullptr;
    const char *post_transfer_encoding_ = nullptr;
    const char *post_data_ = nullptr;
    uint64_t post_data_offset_ = 0;
    size_t post_data_size_ = 0;
    bool end_of_body_ = false;
    
    // boost::coroutines2::coroutine<std::string_view>::push_type* coroutine_;
    ctx::continuation continuation_, continuation_sink_;
    std::function<void()> response_body_write_;
  };

  typedef std::map<std::string, std::function<void(mhd_ctx&)>> mhd_api;

  struct mhd_server {

    // Build a server.
    template <typename... O> mhd_server(O... opts);

    // Serve a given HTTP api.
    inline void serve(const mhd_api& api);

    // Stop the server.
    inline void stop() { if (daemon_) MHD_stop_daemon(daemon_); daemon_ = nullptr; }
    
  private:
    int port_;
    bool one_thread_per_connection_;
    bool linux_epoll_;
    bool select_;
    int nthreads_;
    std::string https_key_path_;
    std::string https_cert_path_;
    MHD_Daemon* daemon_ = nullptr;
  };

  namespace impl
  {
    auto route_lookup(const mhd_api& api, const char* url)
    {
      auto it = api.find(std::string(url));
      if (it == api.end())
        throw std::runtime_error("route " + std::string(url) + " not found.");
      return it->second;
    }

    int mhd_postdataiterator (void *cls,
                              enum MHD_ValueKind kind,
                              const char *key,
                              const char *filename,
                              const char *content_type,
                              const char *transfer_encoding,
                              const char *data,
                              uint64_t off,
                              size_t size)
    {
      // std::cout << "postdataiter " << (int*)data << std::endl;
      mhd_ctx* pp = (mhd_ctx*)cls;
      if (pp->done_) return MHD_NO;
      pp->post_key_ = key;
      pp->post_filename_ = filename;
      pp->post_content_type_ = content_type;
      pp->post_transfer_encoding_ = transfer_encoding;
      pp->post_data_ = data;
      pp->post_data_offset_ = off;
      pp->post_data_size_ = size;
      pp->continuation_ = pp->continuation_.resume();
      return MHD_YES;
    }
    
    int mhd_handler(void * cls,
                    struct MHD_Connection * connection,
                    const char * url,
                    const char * method,
                    const char * version,
                    const char * upload_data,
                    size_t * upload_data_size,
                    void ** ptr)
    {
      MHD_Response* response = nullptr;

      mhd_ctx* pp = (mhd_ctx*)*ptr;
      if (!pp) // First call to handler.
      {
        pp = new mhd_ctx;
        *ptr = pp;

        pp->connection_ = connection;
        pp->method_ = method;
        pp->http_version_ = version;

        // Run handler.
        pp->continuation_ = ctx::callcc([pp, cls, url](ctx::continuation && sink) mutable {
            pp->continuation_sink_ = std::move(sink);
            route_lookup(*(mhd_api*)cls, url)(*pp);
            pp->done_ = true;
            return std::move(pp->continuation_sink_);
          });
        
        // std::cout << "done: "<< pp->done_ << std::endl;
        if (!pp->done_)       
          pp->post_processor_ = MHD_create_post_processor(pp->connection_, 65536, &mhd_postdataiterator, (void*) pp);
        // std::cout << "post_processor_" << pp->post_processor_ << std::endl;
        
        //route_lookup(*(mhd_api*)cls, url)(*pp);
        return MHD_YES;
      }

      // std::cout << "second  call done: "<< pp->done_ << std::endl;
      
      if (*upload_data_size)
      {
         // std::cout << "*upload_data_size " << *upload_data_size << std::endl;
        if (!pp->done_ and pp->post_processor_)
          MHD_post_process(pp->post_processor_, upload_data, *upload_data_size);
        *upload_data_size = 0;
        return MHD_YES;
      }
      else
      {
        pp->end_of_body_ = true;
        if (!pp->done_ and pp->post_processor_)
        {
          // std::cout << "resume" << std::endl;
          pp->continuation_ = pp->continuation_.resume();
        }
      }
      
      if (!pp->response_)
        pp->response_ = MHD_create_response_from_buffer(16, (void*)"Hello from mhdpp", MHD_RESPMEM_PERSISTENT);

      int ret = MHD_queue_response(connection, pp->response_status_, pp->response_);

      // std::cout << "last call done: "<< pp->done_ << std::endl;
      
      MHD_destroy_response(pp->response_);
      MHD_destroy_post_processor(pp->post_processor_);
      delete pp;
      return MHD_YES;
    }

  }

  template <typename... O>
  mhd_server::mhd_server(O... options)
  {
    auto opts = make_metamap(options...);

    one_thread_per_connection_ = has_key(opts, s::one_thread_per_connection);
    linux_epoll_ = has_key(opts, s::linux_epoll);
    select_ = has_key(opts, s::select);
    nthreads_ = get_or(opts, s::nthreads, 1);
    https_key_path_ = get_or(opts, s::https_key_path, "");
    https_cert_path_ = get_or(opts, s::https_cert_path, "");

    static_assert(has_key<decltype(opts)>(s::port), "microhttpd_server: missing required s::port option.");
    port_ = opts.port;
  }
    
  void mhd_server::serve(const mhd_api& api)
  {
    int flags = MHD_USE_SELECT_INTERNALLY;
    if (one_thread_per_connection_)
      flags = MHD_USE_THREAD_PER_CONNECTION;
    else if (select_)
      flags = MHD_USE_SELECT_INTERNALLY;
    else if (linux_epoll_)
    {
#if MHD_VERSION >= 0x00095100
      flags = MHD_USE_EPOLL_INTERNALLY;
#else
      flags = MHD_USE_EPOLL_INTERNALLY_LINUX_ONLY;
#endif
    }


    auto read_file = [] (std::string path) {
      std::ifstream in(path);
      if (!in.good())
      {
        std::stringstream err_ss;
        err_ss << "Error: Cannot read " << path << " " << strerror(errno);
        throw std::runtime_error(err_ss.str());
      }
      std::ostringstream ss{};
      ss << in.rdbuf();
      return ss.str();
    };

    std::string https_cert, https_key;
    if (https_cert_path_.size() || https_key_path_.size())
    {
#ifdef MHD_USE_TLS
      flags |= MHD_USE_TLS;
#else
      flags |= MHD_USE_SSL;
#endif
      
      if (https_cert_path_.size() == 0) throw std::runtime_error("Missing HTTPS certificate file"); 
      if (https_key_path_.size() == 0) throw std::runtime_error("Missing HTTPS key file");

      https_key = read_file(https_key_path_);
      https_cert = read_file(https_cert_path_);

    }

    char* https_cert_buffer = https_cert.size() ? strdup(https_cert.c_str()) : 0;
    char* https_key_buffer = https_key.size() ? strdup(https_key.c_str()) : 0;

    int options[20];
    for (int i = 0; i < 20; i++) options[i] = MHD_OPTION_END;

    options[0] = MHD_OPTION_THREAD_POOL_SIZE;
    options[1] = nthreads_;
    daemon_ = MHD_start_daemon(flags, port_, NULL, NULL,
                               &impl::mhd_handler,
                               (void*)&api,
                               options[0], options[1],
                               options[2], options[3],
                               options[4], options[5]);
    
  }

}
