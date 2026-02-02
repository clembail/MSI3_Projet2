#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include "context.hxx"
#include <iostream>

Context* Context::_sInstance = nullptr;

Context* Context::instance(){
  if (_sInstance == nullptr){
    _sInstance = new Context();
  }
  return _sInstance;
}

Context::Context(){
  _mDevice = MTL::CreateSystemDefaultDevice();
  if (_mDevice==nullptr){
    std::cout << "Failed to create Metal device" << std::endl;
    exit(1);
  }

  _mCommandQueue = _mDevice->newCommandQueue();
  if (_mCommandQueue==nullptr){
    std::cout << "Failed to create Metal command queue" << std::endl;
    exit(1);
  }
}

Context::~Context(){
  if (_mDevice) _mDevice -> release();
  if (_mCommandQueue) _mCommandQueue -> release();
  _sInstance = nullptr;
}

MTL::Device* Context::getDevice(){
  return _mDevice;
}

MTL::CommandQueue* Context::getCommandQueue(){
  return _mCommandQueue;
}
