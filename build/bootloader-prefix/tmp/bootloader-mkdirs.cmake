# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "K:/ESP-IDF/Espressif/frameworks/esp-idf-v5.1.1/components/bootloader/subproject"
  "K:/ESP-IDF/PROJECT/Sleep_Airsense/build/bootloader"
  "K:/ESP-IDF/PROJECT/Sleep_Airsense/build/bootloader-prefix"
  "K:/ESP-IDF/PROJECT/Sleep_Airsense/build/bootloader-prefix/tmp"
  "K:/ESP-IDF/PROJECT/Sleep_Airsense/build/bootloader-prefix/src/bootloader-stamp"
  "K:/ESP-IDF/PROJECT/Sleep_Airsense/build/bootloader-prefix/src"
  "K:/ESP-IDF/PROJECT/Sleep_Airsense/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "K:/ESP-IDF/PROJECT/Sleep_Airsense/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "K:/ESP-IDF/PROJECT/Sleep_Airsense/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
