#include <audioclient.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <endpointvolume.h>

#include <iostream>
#include <string>
#include <thread>

static float old_volume = 0.0f;
static int sessionCountOld = 0;

void syncMasterVolumeToAllApps(IAudioEndpointVolume *endpointVolume, IAudioSessionEnumerator *sessionEnum)
{
  float volume = 0.0f;
  endpointVolume->GetChannelVolumeLevelScalar(0, &volume);

  int sessionCount;
  sessionEnum->GetCount(&sessionCount);

  if(old_volume != volume || sessionCount != sessionCountOld)
  {
    std::cout << "Sync to Mastervolume: " << volume << std::endl;

    for (int i = 0; i < sessionCount; i++)
    {
      IAudioSessionControl *sessionControl;
      ISimpleAudioVolume *audioVolume;


      sessionEnum->GetSession(i, &sessionControl);
      sessionControl->QueryInterface(__uuidof(ISimpleAudioVolume), (void **) &audioVolume);

      audioVolume->SetMasterVolume(volume, 0);

      audioVolume->Release();
      sessionControl->Release();
    }

    old_volume = volume;
    sessionCountOld = sessionCount;
  }
}

int main() {
  CoInitialize(nullptr);

  std::cout << "Volsync v1.2" << std::endl;

  IMMDevice *mmDevice;
  IMMDeviceEnumerator *mmDeviceEnum;
  IAudioSessionManager2 *sessionManager;
  IAudioSessionEnumerator *sessionEnum;
  IAudioEndpointVolume *endpointVolume;

  CoCreateInstance(__uuidof(MMDeviceEnumerator), 0, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&mmDeviceEnum);
  mmDeviceEnum->GetDefaultAudioEndpoint(eRender, eMultimedia, &mmDevice);

  mmDevice->Activate(__uuidof(IAudioEndpointVolume) , CLSCTX_ALL, 0, (void**)&endpointVolume);
  mmDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, 0, (void**)&sessionManager);

  while(true)
  {
    sessionManager->GetSessionEnumerator(&sessionEnum);
    syncMasterVolumeToAllApps(endpointVolume, sessionEnum);
    sessionEnum->Release();
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);
  }
  return 0;
}
