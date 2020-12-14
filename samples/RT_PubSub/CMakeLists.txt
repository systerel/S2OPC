cmake_minimum_required (VERSION 3.5)

project (S2OPC_PubSub_Samples LANGUAGES C)

if(ENABLE_SAMPLES)
  # Demo RT PubSub server
  add_definitions(-D_GNU_SOURCE)
  add_executable(udp_rt_pub "udp_rt_pub.c")
  target_compile_options(udp_rt_pub PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(udp_rt_pub PRIVATE ${S2OPC_DEFINITIONS})
  target_link_libraries(udp_rt_pub PRIVATE s2opc_pubsub)

endif(ENABLE_SAMPLES)
