  set(source_files
         include/dlib/base64/base64_kernel_1.cpp
         include/dlib/bigint/bigint_kernel_1.cpp
         include/dlib/bigint/bigint_kernel_2.cpp
         include/dlib/bit_stream/bit_stream_kernel_1.cpp
         include/dlib/entropy_decoder/entropy_decoder_kernel_1.cpp
         include/dlib/entropy_decoder/entropy_decoder_kernel_2.cpp
         include/dlib/entropy_encoder/entropy_encoder_kernel_1.cpp
         include/dlib/entropy_encoder/entropy_encoder_kernel_2.cpp
         include/dlib/md5/md5_kernel_1.cpp
         include/dlib/tokenizer/tokenizer_kernel_1.cpp
         include/dlib/unicode/unicode.cpp
         include/dlib/data_io/image_dataset_metadata.cpp)

  set(source_files ${source_files}
         include/dlib/sockets/sockets_kernel_1.cpp
         include/dlib/bsp/bsp.cpp
         include/dlib/dir_nav/dir_nav_kernel_1.cpp
         include/dlib/dir_nav/dir_nav_kernel_2.cpp
         include/dlib/dir_nav/dir_nav_extensions.cpp
         include/dlib/linker/linker_kernel_1.cpp
         include/dlib/logger/extra_logger_headers.cpp
         include/dlib/logger/logger_kernel_1.cpp
         include/dlib/logger/logger_config_file.cpp
         include/dlib/misc_api/misc_api_kernel_1.cpp
         include/dlib/misc_api/misc_api_kernel_2.cpp
         include/dlib/sockets/sockets_extensions.cpp
         include/dlib/sockets/sockets_kernel_2.cpp
         include/dlib/sockstreambuf/sockstreambuf.cpp
         include/dlib/sockstreambuf/sockstreambuf_unbuffered.cpp
         include/dlib/server/server_kernel.cpp
         include/dlib/server/server_iostream.cpp
         include/dlib/server/server_http.cpp
         include/dlib/threads/multithreaded_object_extension.cpp
         include/dlib/threads/threaded_object_extension.cpp
         include/dlib/threads/threads_kernel_1.cpp
         include/dlib/threads/threads_kernel_2.cpp
         include/dlib/threads/threads_kernel_shared.cpp
         include/dlib/threads/thread_pool_extension.cpp
         include/dlib/timer/timer.cpp
         include/dlib/stack_trace.cpp
         )

add_library(dlib STATIC ${source_files} )
