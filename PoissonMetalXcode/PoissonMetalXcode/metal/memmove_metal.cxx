#include <Metal/Metal.hpp>
#include "memmove.hxx"
#include "Context.hxx"
#include "SharedStructs.h"
#include <iostream>
#include <cstring>
#include <map>

static std::map<void*, MTL::Buffer*> _buffers;

double* allocate(int n){
  size_t size = n*sizeof(double);
  MTL::Buffer* buffer = Context::instance()
    ->getDevice()
    ->newBuffer(size, MTL::ResourceStorageModeShared);
  void* ptr = buffer->contents();
  _buffers[ptr] = buffer;
  return static_cast<double*>(ptr);
}

void deallocate(double *&d){
  void* ptr = static_cast<void*>(d);

  if (_buffers.find(ptr) != _buffers.end()){
    MTL::Buffer* buffer = _buffers[ptr];
    buffer -> release();
    buffer = nullptr;
    _buffers.erase(ptr);
  }

  d = nullptr;
}

MTL::Buffer* getMetalBuffer(void* ptr){
  if (_buffers.find(ptr) != _buffers.end()){
    MTL::Buffer* buffer = _buffers[ptr];
    return buffer;
  }
  else {
    std::cerr << "Buffer not found" << std::endl;
    return nullptr;
  }
}

void copyDeviceToHost(double *h, double *d, int n){
  size_t size = n*sizeof(double);
  std::memcpy(h, d, size);
}

void copyHostToDevice(double *h, double *d, int n){
  size_t size = n*sizeof(double);
  std::memcpy(d, h, size);
}

void copyDeviceToDevice(double *d_out, double *d_in, int n){
  size_t size = n*sizeof(double);
  MTL::Buffer* buffer_in = getMetalBuffer((void*)d_in);
  MTL::Buffer* buffer_out = getMetalBuffer((void*)d_out);
  if (!buffer_in || !buffer_out) {
    std::cerr << "Buffer not found" << std::endl;
    return;
  }

  auto context = Context::instance();
  MTL::CommandQueue* queue = context -> getCommandQueue();
  MTL::CommandBuffer* command = queue -> commandBuffer();

  MTL::BlitCommandEncoder* blitEncoder = command -> blitCommandEncoder();
  if (!blitEncoder) return;
  blitEncoder -> copyFromBuffer(buffer_in, 0, buffer_out, 0, size);
  blitEncoder -> endEncoding();
  command -> commit();
  command -> waitUntilCompleted();
}
