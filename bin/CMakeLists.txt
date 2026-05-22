add_executable(
    itmoloops
    main.cpp
)

target_link_libraries(itmoloops PRIVATE itmoloops_lib)

if (WIN32)
  target_link_options(itmoloops PRIVATE
    "LINKER:/WHOLEARCHIVE:$<TARGET_FILE:itmoloops_lib>"
  )
endif()
if (MSVC)
  target_link_options(itmoloops PRIVATE
    "LINKER:/WHOLEARCHIVE:$<TARGET_FILE:itmoloops_lib>"
  )
elseif(APPLE)
  target_link_options(itmoloops PRIVATE
    "-Wl,-force_load,$<TARGET_FILE:itmoloops_lib>"
  )
elseif(UNIX)
  target_link_options(itmoloops PRIVATE
    "-Wl,--whole-archive"
    "$<TARGET_FILE:itmoloops_lib>"
    "-Wl,--no-whole-archive"
  )
endif()