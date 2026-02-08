#include "dim.hxx"
#include "variation.hxx"
#include "values.hxx"
#include "memmove.hxx"
#include "Context.hxx"
#include <Metal/Metal.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include "SharedStructs.h"


void allocVariationData(
  float *& diff,
  int n,
  float *& diffPartial,
  int nPartial
)
{
  if (diff == nullptr) diff = allocate(n);
  if (diffPartial == nullptr) diffPartial = allocate(nPartial);
}


void freeVariationData(
  float *& diff,
  float *& diffPartial
)
{
  deallocate(diff);
  deallocate(diffPartial);
}


float variation(
  const Values &u,
  const Values &v,
  float *&diff,
  float *&diffPartial,
  int n
)
{
  int GroupSizeVal = 256;
  int nbGroups = (n+GroupSizeVal-1)/GroupSizeVal;

  allocVariationData(diff, n, diffPartial, nbGroups);

  MTL::Device* device = Context::instance() -> getDevice();
  MTL::CommandQueue* queue = Context::instance() -> getCommandQueue();
  MTL::CommandBuffer* commandBuffer = queue -> commandBuffer();

  {
    static MTL::ComputePipelineState* psoDiff = nullptr;
    if (psoDiff == nullptr){
      MTL::Library* library = device -> newDefaultLibrary();
      if (library == nullptr){
        std::cerr << "Failed to create Metal library";
        return 0.0;
      }
      NS::String* functionName = NS::String::string("k_difference", NS::UTF8StringEncoding);
      MTL::Function* function = library -> newFunction(functionName);
      NS::Error* error = nullptr;
      psoDiff = device -> newComputePipelineState(function, &error);
      if (error != nullptr){
        std::cerr << "Failed to create Metal pipeline state: "
        << error -> localizedDescription() -> utf8String()
        << std::endl;
        return 0.0;
      }
      function -> release();
      library -> release();
    }

    MTL::ComputeCommandEncoder* encoder = commandBuffer -> computeCommandEncoder();
    encoder -> setComputePipelineState(psoDiff);

    MTL::Buffer* bufferU = getMetalBuffer((void*)u.dataGPU());
    MTL::Buffer* bufferV = getMetalBuffer((void*)v.dataGPU());
    MTL::Buffer* bufferDiff = getMetalBuffer(diff);

    encoder -> setBuffer(bufferU, 0, 0);
    encoder -> setBuffer(bufferV, 0, 1);
    encoder -> setBuffer(bufferDiff, 0, 2);
    encoder -> setBytes(&n, sizeof(int), 3);

    MTL::Size groupSize = MTL::Size::Make(GroupSizeVal, 1, 1);
    MTL::Size gridSize = MTL::Size::Make(nbGroups, 1, 1);

    encoder -> dispatchThreadgroups(gridSize, groupSize);
    encoder -> endEncoding();
  }

  {
    static MTL::ComputePipelineState* psoReduce = nullptr;
    if (psoReduce == nullptr){
      MTL::Library* library = device -> newDefaultLibrary();
      if (library == nullptr){
        std::cerr << "Failed to create Metal library";
        return 0.0;
      }
      NS::String* functionName = NS::String::string("k_reduce", NS::UTF8StringEncoding);
      MTL::Function* function = library -> newFunction(functionName);
      NS::Error* error = nullptr;
      psoReduce = device -> newComputePipelineState(function, &error);
      if (error != nullptr){
        std::cerr << "Failed to create Metal pipeline state: "
        << error -> localizedDescription() -> utf8String()
        << std::endl;
        return 0.0;
      }
      function -> release();
      library -> release();
    }

    MTL::ComputeCommandEncoder* encoder = commandBuffer -> computeCommandEncoder();
    encoder -> setComputePipelineState(psoReduce);

    MTL::Buffer* bufferDiff = getMetalBuffer(diff);
    MTL::Buffer* bufferDiffPartial = getMetalBuffer(diffPartial);

    encoder -> setBuffer(bufferDiff, 0, 0);
    encoder -> setBuffer(bufferDiffPartial, 0, 1);
    encoder->setBytes(&n, sizeof(int), 2);

    encoder -> setThreadgroupMemoryLength(GroupSizeVal*sizeof(float),0);

    MTL::Size groupSize = MTL::Size::Make(GroupSizeVal, 1, 1);
    MTL::Size gridSize = MTL::Size::Make(nbGroups, 1, 1);

    encoder -> dispatchThreadgroups(gridSize, groupSize);
    encoder -> endEncoding();
  }

  commandBuffer -> commit();
  commandBuffer -> waitUntilCompleted();

  // sur CPU

  std::vector<float> h_partial(nbGroups);
  copyDeviceToHost(h_partial.data(), diffPartial, nbGroups);

  float totalSum = 0.0;
  for (float value : h_partial){
    totalSum += value;
  }

  return totalSum;
}
