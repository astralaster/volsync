#include <audioclient.h>
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>
#include <audiopolicy.h>
#include <endpointvolume.h>

#include <iostream>
#include <mmeapi.h>
#include <string>
#include <cwchar>
#include <thread>

static float old_volume = 0.0f;

void syncMasterVolumeToAllApps()
{
  IMMDevice *mmDevice;
  IMMDeviceEnumerator *mmDeviceEnum;
  IAudioSessionManager2 *sessionManager;
  IAudioSessionEnumerator *sessionEnum;
  IAudioSessionControl *sessionControl;
  ISimpleAudioVolume *audioVolume;
  IAudioEndpointVolume *endpointVolume;

  CoCreateInstance(__uuidof(MMDeviceEnumerator), 0, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&mmDeviceEnum);
  mmDeviceEnum->GetDefaultAudioEndpoint(eRender, eMultimedia, &mmDevice);

  mmDevice->Activate(__uuidof(IAudioEndpointVolume) , CLSCTX_ALL, 0, (void**)&endpointVolume);
  float volume = 0.0f;
  endpointVolume->GetChannelVolumeLevelScalar(0, &volume);

  if(old_volume != volume) {
    mmDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, 0, (void **) &sessionManager);
    std::cout << "Sync to Mastervolume: " << volume << std::endl;


    sessionManager->GetSessionEnumerator(&sessionEnum);

    int sessionCount;
    sessionEnum->GetCount(&sessionCount);
    for (int i = 0; i < sessionCount; i++) {
      sessionEnum->GetSession(i, &sessionControl);
      sessionControl->QueryInterface(__uuidof(ISimpleAudioVolume), (void **) &audioVolume);

      audioVolume->SetMasterVolume(volume, 0);

      audioVolume->Release();
      sessionControl->Release();
    }

    sessionEnum->Release();
    sessionManager->Release();
    old_volume = volume;
  }
  endpointVolume->Release();
  mmDevice->Release();
  mmDeviceEnum->Release();
}

int main() {
  CoInitialize(nullptr);

  std::cout << "Volsync v1.0" << std::endl;
  while(true)
  {
    syncMasterVolumeToAllApps();
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);
  }
  return 0;
}
