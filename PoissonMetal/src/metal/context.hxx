#pragma once
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>

class Context {
public:
  static Context* instance();

  MTL::Device* getDevice();
  MTL::CommandQueue* getCommandQueue();

private:
  Context();
  ~Context();

  MTL::Device *_mDevice;
  MTL::CommandQueue *_mCommandQueue;

  static Context* _sInstance;
};
