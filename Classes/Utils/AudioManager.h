// AudioManager.h
#ifndef COC_AUDIO_MANAGER_H
#define COC_AUDIO_MANAGER_H

#include <string>
#include <unordered_map>

class AudioManager {
 public:
  static AudioManager* getInstance();
  static void destroyInstance();

  void preload(const std::string& file);
  int playEffect(const std::string& file, float volume = 1.0f,
                 bool loop = false);
  int playMusic(const std::string& file, float volume = 1.0f, bool loop = true);
  void stop(int audioID);
  void stopAll();
  void setVolume(int audioID, float volume);

 private:
  AudioManager();
  ~AudioManager();

  std::unordered_map<std::string, int> _lastPlayIds;
};

#endif  // COC_AUDIO_MANAGER_H
