#pragma once

// Structures partagées entre C++ et Metal
struct constants {
  float d_xmin[3];
  float d_dx[3];
  float d_lambda[3];
  float d_dt;
  int d_n[3];
  int padding; // Alignement
};

// Code UNIQUEMENT pour C++ (pas pour Metal)
#ifndef __METAL_VERSION__

#include <Metal/Metal.hpp>

MTL::Buffer* getMetalBuffer(void* ptr);

// AJOUT IMPORTANT : La déclaration partagée
//constants& getConstants();

#endif  // __METAL_VERSION__
