/* Independent Vulkan validation reproducer: no Autodesk data, shaders or draws.
 * Modes: default (classic bind between offsets), --no-classic, --rebind.
 * Requires Vulkan 1.3, VK_EXT_descriptor_buffer and Khronos validation. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>
static unsigned errors, target_errors;
static VKAPI_ATTR VkBool32 VKAPI_CALL
message(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT type,
        const VkDebugUtilsMessengerCallbackDataEXT *data, void *user) {
  (void)type;
  (void)user;
  if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    errors++;
    if (data->pMessageIdName && strstr(data->pMessageIdName, "08065"))
      target_errors++;
    fprintf(stderr, "%s: %s\n", data->pMessageIdName, data->pMessage);
  }
  return VK_FALSE;
}
#define CHECK(expr)                                                            \
  do {                                                                         \
    VkResult result = (expr);                                                  \
    if (result != VK_SUCCESS) {                                                \
      fprintf(stderr, "%s failed: %d\n", #expr, result);                       \
      exit(2);                                                                 \
    }                                                                          \
  } while (0)
int main(int argc, char **argv) {
  int classic = 1, rebind = 0;
  if (argc == 2 && !strcmp(argv[1], "--no-classic"))
    classic = 0;
  else if (argc == 2 && !strcmp(argv[1], "--rebind"))
    rebind = 1;
  else if (argc != 1)
    return 2;
  VkApplicationInfo app = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                           .pApplicationName = "descriptor-rebind-repro",
                           .apiVersion = VK_API_VERSION_1_3};
  const char *layer = "VK_LAYER_KHRONOS_validation",
             *ext = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
  VkDebugUtilsMessengerCreateInfoEXT debug = {
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
      .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT,
      .pfnUserCallback = message};
  VkInstanceCreateInfo ici = {.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                              .pNext = &debug,
                              .pApplicationInfo = &app,
                              .enabledLayerCount = 1,
                              .ppEnabledLayerNames = &layer,
                              .enabledExtensionCount = 1,
                              .ppEnabledExtensionNames = &ext};
  VkInstance instance;
  CHECK(vkCreateInstance(&ici, 0, &instance));
  PFN_vkCreateDebugUtilsMessengerEXT create_debug =
      (void *)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
  PFN_vkDestroyDebugUtilsMessengerEXT destroy_debug =
      (void *)vkGetInstanceProcAddr(instance,
                                    "vkDestroyDebugUtilsMessengerEXT");
  VkDebugUtilsMessengerEXT messenger;
  CHECK(create_debug(instance, &debug, 0, &messenger));
  uint32_t count = 0;
  CHECK(vkEnumeratePhysicalDevices(instance, &count, 0));
  if (!count)
    return 2;
  VkPhysicalDevice *physical = calloc(count, sizeof(*physical));
  CHECK(vkEnumeratePhysicalDevices(instance, &count, physical));
  VkPhysicalDevice gpu = VK_NULL_HANDLE;
  uint32_t family = 0;
  for (uint32_t i = 0; i < count && !gpu; i++) {
    VkPhysicalDeviceDescriptorBufferFeaturesEXT df = {
        .sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT};
    VkPhysicalDeviceVulkan12Features f12 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .pNext = &df};
    VkPhysicalDeviceFeatures2 f = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &f12};
    vkGetPhysicalDeviceFeatures2(physical[i], &f);
    if (!df.descriptorBuffer || !f12.bufferDeviceAddress)
      continue;
    uint32_t n = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical[i], &n, 0);
    VkQueueFamilyProperties *q = calloc(n, sizeof(*q));
    vkGetPhysicalDeviceQueueFamilyProperties(physical[i], &n, q);
    for (uint32_t j = 0; j < n; j++)
      if (q[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        gpu = physical[i];
        family = j;
        break;
      }
    free(q);
  }
  free(physical);
  if (!gpu) {
    fprintf(stderr, "No suitable GPU\n");
    return 2;
  }
  VkPhysicalDeviceProperties props;
  vkGetPhysicalDeviceProperties(gpu, &props);
  printf("device=%s\n", props.deviceName);
  float priority = 1;
  VkDeviceQueueCreateInfo qci = {.sType =
                                     VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                 .queueFamilyIndex = family,
                                 .queueCount = 1,
                                 .pQueuePriorities = &priority};
  VkPhysicalDeviceDescriptorBufferFeaturesEXT df = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT,
      .descriptorBuffer = VK_TRUE};
  VkPhysicalDeviceVulkan12Features f12 = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
      .pNext = &df,
      .bufferDeviceAddress = VK_TRUE};
  const char *device_ext = VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME;
  VkDeviceCreateInfo dci = {.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                            .pNext = &f12,
                            .queueCreateInfoCount = 1,
                            .pQueueCreateInfos = &qci,
                            .enabledExtensionCount = 1,
                            .ppEnabledExtensionNames = &device_ext};
  VkDevice device;
  CHECK(vkCreateDevice(gpu, &dci, 0, &device));
  PFN_vkCmdBindDescriptorBuffersEXT bind =
      (void *)vkGetDeviceProcAddr(device, "vkCmdBindDescriptorBuffersEXT");
  PFN_vkCmdSetDescriptorBufferOffsetsEXT offsets =
      (void *)vkGetDeviceProcAddr(device, "vkCmdSetDescriptorBufferOffsetsEXT");
  PFN_vkGetDescriptorSetLayoutSizeEXT layout_size =
      (void *)vkGetDeviceProcAddr(device, "vkGetDescriptorSetLayoutSizeEXT");
  if (!bind || !offsets || !layout_size)
    return 2;
  VkDescriptorSetLayoutBinding binding = {
      .binding = 0,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT};
  VkDescriptorSetLayoutCreateInfo lci = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,
      .bindingCount = 1,
      .pBindings = &binding};
  VkDescriptorSetLayout layouts[2];
  CHECK(vkCreateDescriptorSetLayout(device, &lci, 0, &layouts[0]));
  lci.flags = 0;
  CHECK(vkCreateDescriptorSetLayout(device, &lci, 0, &layouts[1]));
  VkPipelineLayout pipeline_layouts[2];
  for (int i = 0; i < 2; i++) {
    VkPipelineLayoutCreateInfo p = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &layouts[i]};
    CHECK(vkCreatePipelineLayout(device, &p, 0, &pipeline_layouts[i]));
  }
  VkDeviceSize size;
  layout_size(device, layouts[0], &size);
  VkBufferCreateInfo bci = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = size,
      .usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT |
               VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE};
  VkBuffer buffer;
  CHECK(vkCreateBuffer(device, &bci, 0, &buffer));
  VkMemoryRequirements req;
  vkGetBufferMemoryRequirements(device, buffer, &req);
  uint32_t mt = 0;
  while (mt < 32 && !(req.memoryTypeBits & (1u << mt)))
    mt++;
  if (mt == 32)
    return 2;
  VkMemoryAllocateFlagsInfo flags = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
      .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT};
  VkMemoryAllocateInfo mai = {.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                              .pNext = &flags,
                              .allocationSize = req.size,
                              .memoryTypeIndex = mt};
  VkDeviceMemory memory;
  CHECK(vkAllocateMemory(device, &mai, 0, &memory));
  CHECK(vkBindBufferMemory(device, buffer, memory, 0));
  VkBufferDeviceAddressInfo address = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .buffer = buffer};
  VkDeviceAddress va = vkGetBufferDeviceAddress(device, &address);
  VkDescriptorPoolSize pool_size = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1};
  VkDescriptorPoolCreateInfo pci = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .maxSets = 1,
      .poolSizeCount = 1,
      .pPoolSizes = &pool_size};
  VkDescriptorPool pool;
  CHECK(vkCreateDescriptorPool(device, &pci, 0, &pool));
  VkDescriptorSetAllocateInfo sai = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = pool,
      .descriptorSetCount = 1,
      .pSetLayouts = &layouts[1]};
  VkDescriptorSet set;
  CHECK(vkAllocateDescriptorSets(device, &sai, &set));
  VkCommandPoolCreateInfo cpci = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .queueFamilyIndex = family};
  VkCommandPool cp;
  CHECK(vkCreateCommandPool(device, &cpci, 0, &cp));
  VkCommandBufferAllocateInfo cai = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = cp,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1};
  VkCommandBuffer cmd;
  CHECK(vkAllocateCommandBuffers(device, &cai, &cmd));
  VkCommandBufferBeginInfo begin = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  CHECK(vkBeginCommandBuffer(cmd, &begin));
  VkDescriptorBufferBindingInfoEXT db = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT,
      .address = va,
      .usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT};
  uint32_t index = 0;
  VkDeviceSize offset = 0;
  bind(cmd, 1, &db);
  offsets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layouts[0], 0, 1,
          &index, &offset);
  printf("errors_before_classic=%u\n", errors);
  if (classic)
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline_layouts[1], 0, 1, &set, 0, 0);
  if (rebind)
    bind(cmd, 1, &db);
  offsets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layouts[0], 0, 1,
          &index, &offset);
  CHECK(vkEndCommandBuffer(cmd));
  vkDestroyCommandPool(device, cp, 0);
  vkDestroyDescriptorPool(device, pool, 0);
  vkDestroyBuffer(device, buffer, 0);
  vkFreeMemory(device, memory, 0);
  for (int i = 0; i < 2; i++) {
    vkDestroyPipelineLayout(device, pipeline_layouts[i], 0);
    vkDestroyDescriptorSetLayout(device, layouts[i], 0);
  }
  vkDestroyDevice(device, 0);
  destroy_debug(instance, messenger, 0);
  vkDestroyInstance(instance, 0);
  printf("classic=%d rebind=%d errors=%u target_08065=%u\n", classic, rebind,
         errors, target_errors);
  return errors ? 1 : 0;
}
