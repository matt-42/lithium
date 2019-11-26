#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <microhttpd.h>

const char* PAGE = "hello world";

struct req_ctx
{
  char* response_str;
  MHD_PostProcessor* post_processor_;
};

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
  req_ctx* ctx = (req_ctx*)cls;

  ctx->response_str = strdup(data);
}

static int
ahc_echo (void *cls,
          struct MHD_Connection *connection,
          const char *url,
          const char *method,
          const char *version,
          const char *upload_data, size_t *upload_data_size, void **ptr)
{
  struct MHD_Response *response;
  int ret;

  /* if (0 != strcmp (method, "POST")) */
  /*   return MHD_NO;              /\* unexpected method *\/ */
  if (!*ptr)
  {
    req_ctx* ctx = (req_ctx*) malloc(sizeof(req_ctx));
    ctx->response_str = 0;
    *ptr = ctx;
    ctx->post_processor_ = MHD_create_post_processor(connection, 65536, &mhd_postdataiterator, (void*) ctx);
    return MHD_YES;
  }

  req_ctx* ctx = (req_ctx*)*ptr;

  if (*upload_data_size)
  {
    MHD_post_process(ctx->post_processor_, upload_data, *upload_data_size);
    *upload_data_size = 0;
    
    return MHD_YES;
  }

  response = MHD_create_response_from_buffer (strlen (ctx->response_str),
					      (void *) ctx->response_str,
					      MHD_RESPMEM_MUST_COPY);
  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);

  if (ctx->response_str)
    free(ctx->response_str);  
  MHD_destroy_response (response);
  MHD_destroy_post_processor (ctx->post_processor_);
  return ret;
}

int
main (int argc, char *const *argv)
{
  struct MHD_Daemon *d;

  if (argc != 2)
    {
      printf ("%s PORT\n", argv[0]);
      return 1;
    }
  d = MHD_start_daemon (MHD_USE_EPOLL_INTERNALLY_LINUX_ONLY,
                        //MHD_USE_THREAD_PER_CONNECTION,
                        atoi (argv[1]),
                        NULL, NULL, &ahc_echo, (void*)PAGE,
                        MHD_OPTION_THREAD_POOL_SIZE, 4,
			MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) 120,
			MHD_OPTION_END);
  if (d == NULL)
    return 1;
  (void) getc (stdin);
  MHD_stop_daemon (d);
  return 0;
}
