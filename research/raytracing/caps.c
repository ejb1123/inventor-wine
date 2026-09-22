#define COBJMACROS
#include <windows.h>
#include <initguid.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#define printf(...) do { char buf[1024]; DWORD n; wsprintfA(buf,__VA_ARGS__); WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),buf,lstrlenA(buf),&n,0); } while(0)
#define CHECK(call) do { HRESULT hr=(call); printf("%s: 0x%08lx\n",#call,(unsigned long)hr); if(FAILED(hr)) return 1; } while(0)
static int probe(void) {
 HRESULT (WINAPI *create)(IUnknown*,D3D_FEATURE_LEVEL,REFIID,void**);
 HRESULT (WINAPI *factoryfn)(REFIID,void**);
 HMODULE d=LoadLibraryA("d3d12.dll"), g=LoadLibraryA("dxgi.dll");
 if(!d||!g) {printf("DLL load error %lu\n",GetLastError());return 1;}
 create=(void*)GetProcAddress(d,"D3D12CreateDevice");factoryfn=(void*)GetProcAddress(g,"CreateDXGIFactory1");
 IDXGIFactory1 *f=NULL; IDXGIAdapter1 *a=NULL; ID3D12Device *dev=NULL;ID3D12CommandQueue *q=NULL;ID3D12Fence *fence=NULL;
 CHECK(factoryfn(&IID_IDXGIFactory1,(void**)&f));
 CHECK(IDXGIFactory1_EnumAdapters1(f,0,&a));
 DXGI_ADAPTER_DESC1 desc;CHECK(IDXGIAdapter1_GetDesc1(a,&desc));printf("Adapter vendor=%04x device=%04x\n",desc.VendorId,desc.DeviceId);
 CHECK(create((IUnknown*)a,D3D_FEATURE_LEVEL_12_0,&IID_ID3D12Device,(void**)&dev));
 D3D12_FEATURE_DATA_D3D12_OPTIONS5 opts={0};
 CHECK(ID3D12Device_CheckFeatureSupport(dev,D3D12_FEATURE_D3D12_OPTIONS5,&opts,sizeof(opts)));
 printf("RaytracingTier=%u (0=unsupported,10=1.0,11=1.1)\n",opts.RaytracingTier);
 D3D12_COMMAND_QUEUE_DESC qd={0};qd.Type=D3D12_COMMAND_LIST_TYPE_DIRECT;
 CHECK(ID3D12Device_CreateCommandQueue(dev,&qd,&IID_ID3D12CommandQueue,(void**)&q));
 CHECK(ID3D12Device_CreateFence(dev,0,D3D12_FENCE_FLAG_NONE,&IID_ID3D12Fence,(void**)&fence));
 HANDLE event=CreateEventA(NULL,FALSE,FALSE,NULL);
 CHECK(ID3D12Fence_SetEventOnCompletion(fence,1,event));CHECK(ID3D12CommandQueue_Signal(q,fence,1));
 DWORD wait=WaitForSingleObject(event,10000);printf("Fence wait=%lu completed=%llu\n",wait,ID3D12Fence_GetCompletedValue(fence));
 CloseHandle(event);ID3D12Fence_Release(fence);ID3D12CommandQueue_Release(q);ID3D12Device_Release(dev);IDXGIAdapter1_Release(a);IDXGIFactory1_Release(f);
 return wait==WAIT_OBJECT_0?0:2;
}

void mainCRTStartup(void) { ExitProcess(probe()); }
