#include "iteration.hxx"
#include "Context.hxx"
#include "SharedStructs.h"
#include "dim.hxx"
#include <iostream>

#include <Metal/Metal.hpp>
#include <Foundation/Foundation.hpp>

void iteration(
    Values & v, Values & u, float dt, int n[3],
    int imin, int imax,
    int jmin, int jmax,
    int kmin, int kmax)
{
  static MTL::ComputePipelineState* pso = nullptr;
  if (pso == nullptr){
    MTL::Device* device = Context::instance() -> getDevice();
    MTL::Library* library = device -> newDefaultLibrary();
    if (library == nullptr){
      std::cerr << "Failed to create Metal library";
      return;
    }
    NS::String* funcName = NS::String::string("k_iteration", NS::UTF8StringEncoding);
    MTL::Function* function = library -> newFunction(funcName);
    NS::Error* error = nullptr;
    pso = device -> newComputePipelineState(function, &error);
    if (pso == nullptr){
      std::cerr << "Failed to create Metal pipeline state"
                << error->localizedDescription()->utf8String()
                << std::endl;
      return;
    }
    function -> release();
    library -> release();
  }

  MTL::Buffer* bufferSource = getMetalBuffer(u.get_d_u());
  MTL::Buffer* bufferTarget = getMetalBuffer(v.get_d_u());
  if (bufferSource == nullptr || bufferTarget == nullptr){
    std::cerr << "Failed to create Metal buffer";
    return;
  }

  constants& cst = getConstants();
  cst.d_dt = dt;
  for (int i = 0; i < 3; i++){
    cst.d_n[i] = n[i];
  }

  MTL::CommandQueue* queue = Context::instance()->getCommandQueue();
  MTL::CommandBuffer* commandBuffer = queue -> commandBuffer();
  MTL::ComputeCommandEncoder* encoder = commandBuffer -> computeCommandEncoder();
  encoder -> setComputePipelineState(pso);

  encoder -> setBuffer(bufferSource, 0, 0);
  encoder -> setBuffer(bufferTarget, 0, 1);
  encoder -> setBytes(&cst, sizeof(cst), 2);

  MTL::Size groupSize = MTL::Size::Make(8,8,4);
  int gridX, gridY, gridZ;
  gridX = (n[0] + groupSize.width - 1)/groupSize.width;
  gridY = (n[1] + groupSize.height - 1)/groupSize.height;
  gridZ = (n[2] + groupSize.depth - 1)/groupSize.depth;
  MTL::Size gridSize = MTL::Size::Make(gridX, gridY, gridZ);

  encoder->dispatchThreadgroups(gridSize, groupSize);

  encoder -> endEncoding();

  commandBuffer -> commit();
  commandBuffer -> waitUntilCompleted();
}
