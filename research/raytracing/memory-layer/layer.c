/* Diagnostic Vulkan layer: records image creation and external-memory pairing.
 * Forwards every API call and does not change parameters or results. */
#include <vulkan/vulkan.h>
#include <vulkan/vk_layer.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
static pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
typedef struct {void *key;VkInstance instance;PFN_vkGetInstanceProcAddr gipa;} Inst;
typedef struct {void *key;PFN_vkGetDeviceProcAddr gdpa;} Dev;
static Inst instances[64];static Dev devices[64];static unsigned ni,nd;
static Inst inst(VkInstance x){Inst r={0};if(!x)return r;void*k=*(void**)x;pthread_mutex_lock(&lock);for(unsigned i=ni;i;i--)if(instances[i-1].key==k){r=instances[i-1];break;}pthread_mutex_unlock(&lock);return r;}
static Dev dev(VkDevice x){Dev r={0};if(!x)return r;void*k=*(void**)x;pthread_mutex_lock(&lock);for(unsigned i=nd;i;i--)if(devices[i-1].key==k){r=devices[i-1];break;}pthread_mutex_unlock(&lock);return r;}
#define NEXT(d,n) ((PFN_vk##n)dev(d).gdpa(d,"vk" #n))
#define LOG(...) do{flockfile(stderr);fprintf(stderr,"MEMTRACE ");fprintf(stderr,__VA_ARGS__);fputc('\n',stderr);funlockfile(stderr);}while(0)
static void identify_fd(const char *operation, int fd) {
 struct stat s;
 if (!fstat(fd, &s)) LOG("fd-identity op=%s fd=%d dev=%llu inode=%llu size=%llu", operation, fd,
  (unsigned long long)s.st_dev, (unsigned long long)s.st_ino, (unsigned long long)s.st_size);
}
VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance,const char*);
VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice,const char*);
VKAPI_ATTR VkResult VKAPI_CALL vkCreateInstance(const VkInstanceCreateInfo*c,const VkAllocationCallbacks*a,VkInstance*out){
 VkLayerInstanceCreateInfo*l=(void*)c->pNext;while(l&&(l->sType!=VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO||l->function!=VK_LAYER_LINK_INFO))l=(void*)l->pNext;
 if(!l||ni>=64)return VK_ERROR_INITIALIZATION_FAILED;
 PFN_vkGetInstanceProcAddr next=l->u.pLayerInfo->pfnNextGetInstanceProcAddr;l->u.pLayerInfo=l->u.pLayerInfo->pNext;
 VkResult r=((PFN_vkCreateInstance)next(0,"vkCreateInstance"))(c,a,out);
 if(r==VK_SUCCESS){pthread_mutex_lock(&lock);instances[ni++]=(Inst){*(void**)*out,*out,next};pthread_mutex_unlock(&lock);}return r;
}
VKAPI_ATTR VkResult VKAPI_CALL vkCreateDevice(VkPhysicalDevice p,const VkDeviceCreateInfo*c,const VkAllocationCallbacks*a,VkDevice*out){
 VkLayerDeviceCreateInfo*l=(void*)c->pNext;while(l&&(l->sType!=VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO||l->function!=VK_LAYER_LINK_INFO))l=(void*)l->pNext;
 if(!l||nd>=64)return VK_ERROR_INITIALIZATION_FAILED;
 PFN_vkGetInstanceProcAddr gipa=l->u.pLayerInfo->pfnNextGetInstanceProcAddr;PFN_vkGetDeviceProcAddr gdpa=l->u.pLayerInfo->pfnNextGetDeviceProcAddr;l->u.pLayerInfo=l->u.pLayerInfo->pNext;
 Inst in=inst((VkInstance)p);if(!in.instance)return VK_ERROR_INITIALIZATION_FAILED;
 VkResult r=((PFN_vkCreateDevice)gipa(in.instance,"vkCreateDevice"))(p,c,a,out);
 if(r==VK_SUCCESS){pthread_mutex_lock(&lock);devices[nd++]=(Dev){*(void**)*out,gdpa};pthread_mutex_unlock(&lock);}return r;
}
VKAPI_ATTR VkResult VKAPI_CALL vkCreateImage(VkDevice d,const VkImageCreateInfo*c,const VkAllocationCallbacks*a,VkImage*out){
 unsigned handles=0;for(const VkBaseInStructure*p=c->pNext;p;p=p->pNext)if(p->sType==VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO)handles=((const VkExternalMemoryImageCreateInfo*)p)->handleTypes;
 VkResult r=NEXT(d,CreateImage)(d,c,a,out);
 if(handles)LOG("image dev=%p image=%p result=%d format=%u extent=%ux%ux%u mips=%u layers=%u samples=%u flags=%x usage=%x tiling=%u layout=%u external=%x",(void*)d,(void*)(r==0?*out:0),r,c->format,c->extent.width,c->extent.height,c->extent.depth,c->mipLevels,c->arrayLayers,c->samples,c->flags,c->usage,c->tiling,c->initialLayout,handles);return r;
}
VKAPI_ATTR VkResult VKAPI_CALL vkAllocateMemory(VkDevice d,const VkMemoryAllocateInfo*c,const VkAllocationCallbacks*a,VkDeviceMemory*out){
 int fd=-1;unsigned exp=0;VkImage image=0;
 for(const VkBaseInStructure*p=c->pNext;p;p=p->pNext){if(p->sType==VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR)fd=((const VkImportMemoryFdInfoKHR*)p)->fd;if(p->sType==VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO)exp=((const VkExportMemoryAllocateInfo*)p)->handleTypes;if(p->sType==VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO)image=((const VkMemoryDedicatedAllocateInfo*)p)->image;}
 if(fd>=0)identify_fd("import",fd);
 VkResult r=NEXT(d,AllocateMemory)(d,c,a,out);if(fd>=0||exp)LOG("memory dev=%p mem=%p result=%d size=%llu type=%u importfd=%d export=%x dedicatedImage=%p",(void*)d,(void*)(r==0?*out:0),r,(unsigned long long)c->allocationSize,c->memoryTypeIndex,fd,exp,(void*)image);return r;
}
VKAPI_ATTR VkResult VKAPI_CALL vkGetMemoryFdKHR(VkDevice d,const VkMemoryGetFdInfoKHR*c,int*out){VkResult r=NEXT(d,GetMemoryFdKHR)(d,c,out);LOG("export dev=%p mem=%p fd=%d result=%d",(void*)d,(void*)c->memory,r==0?*out:-1,r);if(r==0)identify_fd("export",*out);return r;}
VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice d,const char*n){
#define PICK(x) if(!strcmp(n,"vk" #x))return (PFN_vkVoidFunction)vk##x
 PICK(GetDeviceProcAddr);PICK(CreateImage);PICK(AllocateMemory);PICK(GetMemoryFdKHR);
#undef PICK
 Dev x=dev(d);return x.gdpa?x.gdpa(d,n):0;
}
VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance i,const char*n){
#define PICK(x) if(!strcmp(n,"vk" #x))return (PFN_vkVoidFunction)vk##x
 PICK(GetInstanceProcAddr);PICK(GetDeviceProcAddr);PICK(CreateInstance);PICK(CreateDevice);PICK(CreateImage);PICK(AllocateMemory);PICK(GetMemoryFdKHR);
#undef PICK
 Inst x=inst(i);return x.gipa?x.gipa(i,n):0;
}
VKAPI_ATTR VkResult VKAPI_CALL vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface*p){if(p->loaderLayerInterfaceVersion>2)p->loaderLayerInterfaceVersion=2;p->pfnGetInstanceProcAddr=vkGetInstanceProcAddr;p->pfnGetDeviceProcAddr=vkGetDeviceProcAddr;p->pfnGetPhysicalDeviceProcAddr=0;return VK_SUCCESS;}
