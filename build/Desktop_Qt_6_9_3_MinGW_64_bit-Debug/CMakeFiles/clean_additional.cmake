# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\QtMp3Srt_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\QtMp3Srt_autogen.dir\\ParseCache.txt"
  "QtMp3Srt_autogen"
  )
endif()
