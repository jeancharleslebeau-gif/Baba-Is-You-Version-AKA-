cmake_minimum_required(VERSION 3.16)

# Inclure le système de build ESP-IDF
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Déclarer ton projet (nom = babaisu)
project(babaisu)

# Option backend graphique
option(USE_FRAMEBUFFER "Use framebuffer backend" ON)
