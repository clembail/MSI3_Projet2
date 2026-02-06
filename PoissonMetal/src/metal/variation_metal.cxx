#include "variation.hxx"
#include "values.hxx"
#include "memmove.hxx" // Pour allocate/deallocate/copy
#include "Context.hxx"
#include <Metal/Metal.hpp>
#include <iostream>
#include <vector>
#include <cmath> // pour ceil
#include "SharedStructs.h"


void allocVariationData(
  double *& diff,
  int n,
  double *& diffPartial,
  int nPartial
)
{
  if (diff == nullptr) diff = allocate(n);
  if (diffPartial == nullptr) diffPartial = allocate(nPartial);
}


void freeVariationData(
  double *& diff,
  double *& diffPartial
)
{
  deallocate(diff);
  deallocate(diffPartial);
}


double variation(
  const Values &u,
  const Values &v,
  double *&diff,
  double *&diffPartial,
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

    encoder -> setThreadgroupMemoryLength(GroupSizeVal*sizeof(double),0);

    MTL::Size groupSize = MTL::Size::Make(GroupSizeVal, 1, 1);
    MTL::Size gridSize = MTL::Size::Make(nbGroups, 1, 1);

    encoder -> dispatchThreadgroups(gridSize, groupSize);
    encoder -> endEncoding();
  }

  commandBuffer -> commit();
  commandBuffer -> waitUntilCompleted();

  // sur CPU

  std::vector<double> h_partial(nbGroups);
  copyDeviceToHost(h_partial.data(), diffPartial, nbGroups);

  double totalSum = 0.0;
  for (double value : h_partial){
    totalSum += value;
  }

  return totalSum;
}
