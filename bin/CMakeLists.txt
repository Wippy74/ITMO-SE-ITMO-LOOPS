add_executable(
    itmoloops
    main.cpp
)

target_link_libraries(itmoloops PRIVATE itmoloops_lib)

target_link_options(itmoloops PRIVATE
    "LINKER:/WHOLEARCHIVE:$<TARGET_FILE:itmoloops_lib>"
  )
