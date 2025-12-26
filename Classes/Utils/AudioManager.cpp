// AudioManager.cpp
#include "Utils/AudioManager.h"

#include "audio/include/AudioEngine.h"
#include "cocos2d.h"

using namespace cocos2d;

static AudioManager* s_instance = nullptr;

AudioManager* AudioManager::getInstance() {
  if (!s_instance) s_instance = new AudioManager();
  return s_instance;
}

void AudioManager::destroyInstance() {
  if (s_instance) {
    delete s_instance;
    s_instance = nullptr;
  }
}

AudioManager::AudioManager() {}

AudioManager::~AudioManager() { cocos2d::AudioEngine::end(); }

void AudioManager::preload(const std::string& file) {
  std::string path = FileUtils::getInstance()->fullPathForFilename(file);
  cocos2d::AudioEngine::preload(path);
}

int AudioManager::playEffect(const std::string& file, float volume, bool loop) {
  std::string path = FileUtils::getInstance()->fullPathForFilename(file);
  int id = cocos2d::AudioEngine::play2d(path, loop, volume);
  if (id >= 0) {
    _lastPlayIds[file] = id;
  }
  return id;
}

int AudioManager::playMusic(const std::string& file, float volume, bool loop) {
  return playEffect(file, volume, loop);
}

void AudioManager::stop(int audioID) { cocos2d::AudioEngine::stop(audioID); }

void AudioManager::stopAll() { cocos2d::AudioEngine::stopAll(); }

void AudioManager::setVolume(int audioID, float volume) {
  cocos2d::AudioEngine::setVolume(audioID, volume);
}
