#pragma once

#include <jaffarCommon/hash.hpp>
#include <jaffarCommon/exceptions.hpp>
#include <jaffarCommon/file.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/serializers/base.hpp>
#include <jaffarCommon/deserializers/base.hpp>
#include <jaffarCommon/serializers/contiguous.hpp>
#include <jaffarCommon/deserializers/contiguous.hpp>
#include "inputParser.hpp"
#include "libco.h"

#ifdef _ENABLE_RENDERING
#include <SDL.h>
#endif

extern int _main(int argc, char* argv[]);
void runMain() { _main(0, nullptr); }

cothread_t _emuCoroutine;
cothread_t _driverCoroutine;

jaffarCommon::file::MemoryFileDirectory _memFileDirectory; 

namespace jaffar
{

class EmuInstanceBase
{
  public:

  EmuInstanceBase(const nlohmann::json &config)
  {
  }

  virtual ~EmuInstanceBase() 
  {
  }

  void initialize()
  {
    int argc = 0;
    char** argv = nullptr;

    std::string srcDiskFileName = "disk1.ima";
    std::string dstDiskFileName = "FloppyDisk0.ima";
    std::string diskData;
    jaffarCommon::file::loadStringFromFile(diskData, srcDiskFileName);
    auto f = _memFileDirectory.fopen(dstDiskFileName, "w");
    jaffarCommon::file::MemoryFile::fwrite(diskData.data(), diskData.size(), 1, f);
    _memFileDirectory.fclose(f);
    
    f = _memFileDirectory.fopen(dstDiskFileName, "r");
    uint8_t* buf = (uint8_t*) malloc(diskData.size());
    jaffarCommon::file::MemoryFile::fread(buf, diskData.size(), 1, f);
    for (size_t i = 0; i < diskData.size(); i++) if (buf[i] != ((uint8_t*)diskData.data())[i]) JAFFAR_THROW_RUNTIME("Loaded file and mem file differ at %lu (%u vs %u)\n", i, buf[i], ((uint8_t*)diskData.data())[i]);
    free(buf);
    _memFileDirectory.fclose(f);
    
    constexpr size_t stackSize = 4 * 1024 * 1024;
    _emuCoroutine = co_create(stackSize, runMain);
    _driverCoroutine = co_active();
    co_switch(_emuCoroutine);
    // _main(argc, argv);
  }

  virtual void advanceState(const jaffar::input_t &input)
  {
    // Returning to emulator thread to run a single frame
    co_switch(_emuCoroutine);
  }

  inline jaffarCommon::hash::hash_t getStateHash() const
  {
    MetroHash128 hash;

    jaffarCommon::hash::hash_t result;
    hash.Finalize(reinterpret_cast<uint8_t *>(&result));
    return result;
  }


  void printInformation()
  {
  }

  void initializeVideoOutput()
  {
    #ifdef _ENABLE_RENDERING
    SDL_Init(SDL_INIT_VIDEO);
    _renderWindow = SDL_CreateWindow("QuickerDOSBOX-x",  SDL_WINDOWPOS_UNDEFINED,  SDL_WINDOWPOS_UNDEFINED, _videoWidth, _videoHeight, 0);
    _renderer = SDL_CreateRenderer(_renderWindow, -1, SDL_RENDERER_ACCELERATED);
    _texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STREAMING, _videoWidth, _videoHeight);
    #endif
  }

  void finalizeVideoOutput()
  {
    #ifdef _ENABLE_RENDERING
    SDL_DestroyTexture(_texture);
    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_renderWindow);
    SDL_Quit();
    #endif
  }

  void enableRendering()
  {
    #ifdef _ENABLE_RENDERING
    headlessEnableVideoRendering();
    headlessEnableAudioRendering();
    I_SetSoundCap();
    I_InitSound();
    _renderingEnabled = true;
    #endif
  }

  void disableRendering()
  {
    #ifdef _ENABLE_RENDERING
    headlessDisableVideoRendering();
    headlessDisableAudioRendering();
    _renderingEnabled = false;
    #endif
  }

  void updateRenderer()
  {
    #ifdef _ENABLE_RENDERING
    SDL_Rect srcRect  = { 0, 0, _videoWidth, _videoHeight };
    SDL_Rect destRect = { 0, 0, _videoWidth, _videoHeight };

    void *pixels = nullptr;
    int pitch = _videoPitch;
    if (SDL_LockTexture(_texture, nullptr, &pixels, &pitch) < 0) return;
    memcpy(pixels, _videoBuffer, _videoBufferSize);
    SDL_UnlockTexture(_texture);
    SDL_RenderClear(_renderer);
    SDL_RenderCopy(_renderer, _texture, &srcRect, &destRect);
    SDL_RenderPresent(_renderer);
    #endif
  }

  inline size_t getStateSize() const 
  {
    return _stateSize;
  }

  inline jaffar::InputParser *getInputParser() const { return _inputParser.get(); }
  
  void serializeState(jaffarCommon::serializer::Base& s) const
  {
  }

  void deserializeState(jaffarCommon::deserializer::Base& d) 
  {
  }

  size_t getVideoBufferSize() const
  {
    #ifdef _ENABLE_RENDERING
     return _videoBufferSize;
    #endif
     return 0;
  }

  uint8_t* getVideoBufferPtr() const
  {
    #ifdef _ENABLE_RENDERING
     return (uint8_t*)_videoBuffer;
    #endif
     return nullptr;
  }
  
  size_t getEffectiveSaveStateSize() const { return 0; }

  // Virtual functions

  virtual void doSoftReset() = 0;
  virtual void doHardReset() = 0;
  virtual std::string getCoreName() const = 0;

  protected:


  virtual void setWorkRamSerializationSizeImpl(const size_t size) {};
  virtual void enableStateBlockImpl(const std::string& block) {};
  virtual void disableStateBlockImpl(const std::string& block) {};

  // State size
  size_t _stateSize;
  uint8_t* _saveData;

  private:

  std::unique_ptr<jaffar::InputParser> _inputParser;
  static uint32_t InputGetter(void* inputValue) { return *(uint32_t*)inputValue; }

  // Rendering stuff
  #ifdef _ENABLE_RENDERING
  int _videoWidth;
  int _videoHeight;
  int _videoPitch;
  SDL_Window* _renderWindow;
  SDL_Renderer* _renderer;
  SDL_Texture* _texture;
  uint8_t* _videoSource;
  uint32_t* _videoBuffer;
  size_t _videoBufferSize;
  bool _renderingEnabled = false;
  #endif
};

} // namespace jaffar