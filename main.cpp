#include <audioclient.h>
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>

#include <audiopolicy.h>
#include <iostream>
#include <mmeapi.h>
#include <string>

//-----------------------------------------------------------
// This function enumerates all active (plugged in) audio
// rendering endpoint devices. It prints the friendly name
// and endpoint ID string of each endpoint device.
//-----------------------------------------------------------
#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);

void PrintEndpointNames()
{
  HRESULT hr = S_OK;
  IMMDeviceEnumerator *pEnumerator = NULL;
  IMMDeviceCollection *pCollection = NULL;
  IMMDevice *pEndpoint = NULL;
  IPropertyStore *pProps = NULL;
  LPWSTR pwszID = NULL;

  hr = CoCreateInstance(
          CLSID_MMDeviceEnumerator, NULL,
          CLSCTX_ALL, IID_IMMDeviceEnumerator,
          (void**)&pEnumerator);
  EXIT_ON_ERROR(hr)

  hr = pEnumerator->EnumAudioEndpoints(
          eAll, DEVICE_STATE_ACTIVE | DEVICE_STATE_UNPLUGGED,
          &pCollection);
  EXIT_ON_ERROR(hr)

  UINT  count;
  hr = pCollection->GetCount(&count);
  EXIT_ON_ERROR(hr)

  if (count == 0)
  {
    printf("No endpoints found.\n");
  }

  // Each loop prints the name of an endpoint device.
  for (ULONG i = 0; i < count; i++)
  {
    // Get pointer to endpoint number i.
    hr = pCollection->Item(i, &pEndpoint);
    EXIT_ON_ERROR(hr)

    // Get the endpoint ID string.
    hr = pEndpoint->GetId(&pwszID);
    EXIT_ON_ERROR(hr)

    hr = pEndpoint->OpenPropertyStore(
            STGM_READ, &pProps);
    EXIT_ON_ERROR(hr)

    PROPVARIANT varName;
    // Initialize container for property value.
    PropVariantInit(&varName);

    // Get the endpoint's friendly-name property.
    hr = pProps->GetValue(
            PKEY_Device_FriendlyName, &varName);
    EXIT_ON_ERROR(hr)

    // Print endpoint friendly name and endpoint ID.
    DWORD state = 0;
    pEndpoint->GetState(&state);
    printf("Endpoint %d: \"%S\" (%S) state: %d\n",
           i, varName.pwszVal, pwszID, state);

    CoTaskMemFree(pwszID);
    pwszID = NULL;
    PropVariantClear(&varName);
    SAFE_RELEASE(pProps)
    SAFE_RELEASE(pEndpoint)
  }
  SAFE_RELEASE(pEnumerator)
  SAFE_RELEASE(pCollection)
  return;

Exit:
  printf("Error!\n");
  CoTaskMemFree(pwszID);
  SAFE_RELEASE(pEnumerator)
  SAFE_RELEASE(pCollection)
  SAFE_RELEASE(pEndpoint)
  SAFE_RELEASE(pProps)
}

void changeFGWindowVolume()
{
  IMMDevice *mmDevice;
  IMMDeviceEnumerator *mmDeviceEnum;
  IAudioSessionManager2 *sessionManager;
  IAudioSessionEnumerator *sessionEnum;
  IAudioSessionControl *sessionControl;
  IAudioSessionControl2 *sessionControl2;
  ISimpleAudioVolume *audioVolume;

  CoCreateInstance(__uuidof(MMDeviceEnumerator), 0, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&mmDeviceEnum);
  mmDeviceEnum->GetDefaultAudioEndpoint(eRender, eMultimedia, &mmDevice);
  mmDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, 0, (void**)&sessionManager);
  sessionManager->GetSessionEnumerator(&sessionEnum);

  //DWORD activePid;
  //GetWindowThreadProcessId(activeHWND, &activePid);

  int sessionCount;
  sessionEnum->GetCount(&sessionCount);
  for (int i = 0; i < sessionCount; i++)
  {
    sessionEnum->GetSession(i, &sessionControl);
    //sessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&sessionControl2);

    LPWSTR name = NULL;
    sessionControl->GetDisplayName(&name);
    std::wcout << "sessionname: " << name << std::endl;


    sessionControl->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&audioVolume);

    BOOL muted;
    audioVolume->GetMute(&muted);

    float volumeLevel;
    audioVolume->GetMasterVolume(&volumeLevel);

    std::cout << std::to_string(volumeLevel) << std::endl;

    float newVolumeLevel = 33.0 / 100.0f;
    audioVolume->SetMasterVolume(1.0f, 0);
    audioVolume->Release();

    sessionControl->Release();
    //sessionControl2->Release();
  }

  sessionEnum->Release();
  sessionManager->Release();
  mmDevice->Release();
  mmDeviceEnum->Release();
}

int main() {
  CoInitialize(nullptr);

  std::cout << "Hello, World!" << std::endl;
  PrintEndpointNames();
  changeFGWindowVolume();
  return 0;
}
