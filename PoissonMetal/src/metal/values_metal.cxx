#include <Metal/Metal.hpp>
#include <Foundation/Foundation.hpp>
#include "values.hxx"
#include "Context.hxx"
#include "SharedStructs.h"
#include "dim.hxx"

void Values::init()
{
  MTL::Device* device = Context::instance() -> getDevice();
  static MTL::ComputePipelineState* pso = nullptr;
  if (pso == nullptr){
    MTL::Library* library = device -> newDefaultLibrary();
    if (library == nullptr){
      std::cerr << "Failed to create Metal library";
      return;
    }
    NS::String* functionName = NS::String::string("k_init",NS::UTF8StringEncoding);
    MTL::Function* function = library -> newFunction(functionName);
    NS::Error* error = nullptr;
    pso = device -> newComputePipelineState(function, &error);
    if (pso == nullptr){
      std::cerr << "Failed to create Metal pipeline state"
                << error-> localizedDescription() -> utf8String()
                << std::endl;
      return;
    }

    function -> release();
    library -> release();
  }

  MTL::Buffer* bufferSource = getMetalBuffer(d_u);
  if (bufferSource == nullptr){
    std::cerr << "Failed to create Metal buffer";
    return;
  }
  constants& cst = getConstants();

  MTL::CommandQueue* queue = Context::instance() -> getCommandQueue();
  MTL::CommandBuffer* commandBuffer = queue -> commandBuffer();
  MTL::ComputeCommandEncoder* encoder = commandBuffer -> computeCommandEncoder();

  encoder -> setComputePipelineState(pso);
  encoder -> setBuffer(bufferSource,0,0);
  encoder -> setBytes(&cst, sizeof(constants),2);

  MTL::Size groupSize = MTL::Size::Make(8,8,4);
  int gridX, gridY, gridZ;
  gridX = (m_n[0] + groupSize.width - 1)/groupSize.width;
  gridY = (m_n[1] + groupSize.height - 1)/groupSize.height;
  gridZ = (m_n[2] + groupSize.depth - 1)/groupSize.depth;
  MTL::Size gridSize = MTL::Size::Make(gridX, gridY, gridZ);

  encoder -> dispatchThreadgroups(gridSize, groupSize);

  encoder -> endEncoding();

  commandBuffer -> commit();
  commandBuffer -> waitUntilCompleted();
  h_synchronized = false;
};

void Values::boundaries()
{
  MTL::Device* device = Context::instance() -> getDevice();
  static MTL::ComputePipelineState* pso = nullptr;
  if (pso == nullptr){
    MTL::Library* library = device -> newDefaultLibrary();
    if (library == nullptr){
      std::cerr << "Failed to create Metal library";
      return;
    }
    NS::String* functionName = NS::String::string("k_boundaries", NS::UTF8StringEncoding);
    MTL::Function* function = library -> newFunction(functionName);
    NS::Error* error = nullptr;
    pso = device -> newComputePipelineState(function, &error);
    if (pso == nullptr){
      std::cerr << "Failed to create Metal pipeline state"
                << error -> localizedDescription() -> utf8String()
                << std::endl;
      return;
    }
    function -> release();
    library -> release();
  }

  MTL::Buffer* bufferSource = getMetalBuffer(d_u);
  if (bufferSource == nullptr){
    std::cerr << "Failed to create Metal buffer";
    return;
  }

  constants& cst = getConstants();

  MTL::CommandQueue* queue = Context::instance() -> getCommandQueue();
  MTL::CommandBuffer* commandBuffer = queue -> commandBuffer();
  MTL::ComputeCommandEncoder* encoder = commandBuffer -> computeCommandEncoder();

  encoder -> setComputePipelineState(pso);
  encoder -> setBuffer(bufferSource, 0, 0);
  encoder -> setBytes(&cst, sizeof(constants), 2);

  MTL::Size groupSize = MTL::Size::Make(8,8,4);
  int gridX, gridY, gridZ;
  gridX = (m_n[0] + groupSize.width - 1)/groupSize.width;
  gridY = (m_n[1] + groupSize.height - 1)/groupSize.height;
  gridZ = (m_n[2] + groupSize.depth - 1)/groupSize.depth;
  MTL::Size gridSize = MTL::Size::Make(gridX,gridY,gridZ);

  encoder -> dispatchThreadgroups(gridSize, groupSize);

  encoder -> endEncoding();

  commandBuffer -> commit();
  commandBuffer -> waitUntilCompleted();

  h_synchronized = false;
}

void Values::zero()
{
  MTL::CommandQueue* queue = Context::instance() -> getCommandQueue();
  MTL::CommandBuffer* commandBuffer = queue -> commandBuffer();
  MTL::BlitCommandEncoder* blit = commandBuffer -> blitCommandEncoder();

  MTL::Buffer* bufferTarget = getMetalBuffer(d_u);
  if (bufferTarget == nullptr){
    std::cerr << "Failed to create Metal buffer";
    return;
  }

  blit -> fillBuffer(bufferTarget, NS::Range::Make(0, nn*sizeof(double)), 0);
  blit -> endEncoding();

  commandBuffer -> commit();
  commandBuffer -> waitUntilCompleted();

  h_synchronized = false;
}
