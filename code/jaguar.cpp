#include <stdint.h>
#include <stdarg.h>
#include "jaguar.h"

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

// TEMPORARY. GET RID OF ME.
#include <stdlib.h>
#include <math.h>
////////////////////////////

#include "linux.cpp"
#include "memory.cpp"
#include "math.cpp"
#include "library.cpp"

#ifdef DEBUG
	const u8 debug = true;
#else
	const u8 debug = false;
#endif

VkDebugUtilsMessengerEXT vulkan_debug_messenger;
VkInstance vulkan_instance;

#define VK_CHECK(x)\
	do {\
		auto result = (x);\
		if (result != VK_SUCCESS) _abort("VK_CHECK failed on '%s': %s", #x, vk_result_to_string(result));\
	} while (0)

#define VK_EXPORTED_FUNCTION(name)\
	PFN_##name name = NULL;
#define VK_GLOBAL_FUNCTION(name)\
	PFN_##name name = NULL;
#define VK_INSTANCE_FUNCTION(name)\
	PFN_##name name = NULL;
#define VK_DEVICE_FUNCTION(name)\
	PFN_##name name = NULL;

#include "vulkan_functions.h"

#undef VK_EXPORTED_FUNCTION
#undef VK_GLOBAL_FUNCTION
#undef VK_INSTANCE_FUNCTION
#undef VK_DEVICE_FUNCTION

const char *vk_result_to_string(VkResult result) {
	switch(result) {
		case (VK_SUCCESS):
			return "VK_SUCCESS";
		case (VK_NOT_READY):
			return "VK_NOT_READY";
		case (VK_TIMEOUT):
			return "VK_TIMEOUT";
		case (VK_EVENT_SET):
			return "VK_EVENT_SET";
		case (VK_EVENT_RESET):
			return "VK_EVENT_RESET";
		case (VK_INCOMPLETE):
			return "VK_INCOMPLETE";
		case (VK_ERROR_OUT_OF_HOST_MEMORY):
			return "VK_ERROR_OUT_OF_HOST_MEMORY";
		case (VK_ERROR_INITIALIZATION_FAILED):
			return "VK_ERROR_INITIALIZATION_FAILED";
		case (VK_ERROR_DEVICE_LOST):
			return "VK_ERROR_DEVICE_LOST";
		case (VK_ERROR_MEMORY_MAP_FAILED):
			return "VK_ERROR_MEMORY_MAP_FAILED";
		case (VK_ERROR_LAYER_NOT_PRESENT):
			return "VK_ERROR_LAYER_NOT_PRESENT";
		case (VK_ERROR_EXTENSION_NOT_PRESENT):
			return "VK_ERROR_EXTENSION_NOT_PRESENT";
		case (VK_ERROR_FEATURE_NOT_PRESENT):
			return "VK_ERROR_FEATURE_NOT_PRESENT";
		case (VK_ERROR_INCOMPATIBLE_DRIVER):
			return "VK_ERROR_INCOMPATIBLE_DRIVER";
		case (VK_ERROR_TOO_MANY_OBJECTS):
			return "VK_ERROR_TOO_MANY_OBJECTS";
		case (VK_ERROR_FORMAT_NOT_SUPPORTED):
			return "VK_ERROR_FORMAT_NOT_SUPPORTED";
		case (VK_ERROR_FRAGMENTED_POOL):
			return "VK_ERROR_FRAGMENTED_POOL";
		case (VK_ERROR_OUT_OF_DEVICE_MEMORY):
			return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
		case (VK_ERROR_OUT_OF_POOL_MEMORY):
			return "VK_ERROR_OUT_OF_POOL_MEMORY";
		case (VK_ERROR_INVALID_EXTERNAL_HANDLE):
			return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
		case (VK_ERROR_SURFACE_LOST_KHR):
			return "VK_ERROR_SURFACE_LOST_KHR";
		case (VK_ERROR_NATIVE_WINDOW_IN_USE_KHR):
			return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
		case (VK_SUBOPTIMAL_KHR):
			return "VK_SUBOPTIMAL_KHR";
		case (VK_ERROR_OUT_OF_DATE_KHR):
			return "VK_ERROR_OUT_OF_DATE_KHR";
		case (VK_ERROR_INCOMPATIBLE_DISPLAY_KHR):
			return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
		case (VK_ERROR_VALIDATION_FAILED_EXT):
			return "VK_ERROR_VALIDATION_FAILED_EXT";
		case (VK_ERROR_INVALID_SHADER_NV):
			return "VK_ERROR_INVALID_SHADER_NV";
		case (VK_ERROR_NOT_PERMITTED_EXT):
			return "VK_ERROR_NOT_PERMITTED_EXT";
		case (VK_RESULT_RANGE_SIZE):
			return "VK_RESULT_RANGE_SIZE";
		case (VK_RESULT_MAX_ENUM):
			return "VK_RESULT_MAX_ENUM";
	}

	return "Unknown VkResult Code";
}

void cleanup() {
	vkDestroyInstance(vulkan_instance, NULL);
}

u32 vulkan_debug_message_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data) {
	Log_Type log_type;
	switch (severity) {
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: {
		log_type = STANDARD_LOG;
	} break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
		log_type = MINOR_ERROR_LOG;
	} break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
		log_type = CATASTROPHIC_ERROR_LOG;
	} break;
	default: {
		log_type = STANDARD_LOG;
	};
	}

	const char *type_string;
	switch(type) {
	case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: {
		type_string = "General";
	} break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: {
		type_string = "Validation";
	} break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: {
		type_string = "Performance";
	} break;
	default: {
		type_string = "Unknown";
	};
	}

	log_print(log_type, "Vulkan debug message: Message type %s: %s\n", type_string, callback_data->pMessage);
    return false;
}

void application_entry() {
	init_memory();

	Library_Handle vulkan_library = open_shared_library("dependencies/vulkan/1.1.106.0/lib/libvulkan.so");

#define VK_EXPORTED_FUNCTION(name)\
	name = (PFN_##name)dlsym(vulkan_library, (const char *)#name);\
	if (!name) _abort("Failed to load Vulkan function %s.", #name);
#define VK_GLOBAL_FUNCTION(name)\
	name = (PFN_##name)vkGetInstanceProcAddr(NULL, (const char *)#name);\
	if (!name) _abort("Failed to load Vulkan function %s.", #name);
#define VK_INSTANCE_FUNCTION(name)
#define VK_DEVICE_FUNCTION(name)

#include "vulkan_functions.h"

#undef VK_EXPORTED_FUNCTION
#undef VK_GLOBAL_FUNCTION
#undef VK_INSTANCE_FUNCTION
#undef VK_DEVICE_FUNCTION

	u32 layer_count;
	vkEnumerateInstanceLayerProperties(&layer_count, NULL);
	VkLayerProperties *available_layers = (VkLayerProperties *)malloc(sizeof(VkLayerProperties) * layer_count); // @TEMP
	vkEnumerateInstanceLayerProperties(&layer_count, available_layers);

	debug_print("Available Vulkan layers:\n");
	for (s32 i = 0; i < layer_count; i++) {
		debug_print("\t%s\n", available_layers[i].layerName);
	}

	u32 extension_count = 0;
	vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL);
	VkExtensionProperties *extensions = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * extension_count); // @TEMP
	vkEnumerateInstanceExtensionProperties(NULL, &extension_count, extensions);

	debug_print("Available Vulkan extensions:\n");
	for (s32 i = 0; i < extension_count; i++) {
		debug_print("\t%s\n", extensions[i].extensionName);
	}

	VkApplicationInfo app_info = {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "Hello Triangle";
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = "No Engine";
	app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.apiVersion = VK_API_VERSION_1_0;

	// @TODO: Check that all required extensions/layers are available.
	VkInstanceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &app_info;

	s32 instance_extension_count = 0;
	const char *instance_extensions[10];
	instance_extensions[instance_extension_count++] = "VK_KHR_surface";
	instance_extensions[instance_extension_count++] = "VK_KHR_xlib_surface";

	s32 device_extension_count = 0;
	const char *device_extensions[10];
	device_extensions[device_extension_count++] = "VK_KHR_swapchain";

	s32 instance_layer_count;
	const char *instance_layers[10];

	if (debug) {
		instance_extensions[instance_extension_count++] = "VK_EXT_debug_utils";
		instance_layers[instance_layer_count++] = "VK_LAYER_KHRONOS_validation";

		create_info.enabledLayerCount = instance_layer_count;
		create_info.ppEnabledLayerNames = instance_layers;
		create_info.enabledExtensionCount = instance_extension_count;
		create_info.ppEnabledExtensionNames = instance_extensions;
	}

	VK_CHECK(vkCreateInstance(&create_info, NULL, &vulkan_instance));

#define VK_EXPORTED_FUNCTION(name)
#define VK_GLOBAL_FUNCTION(name)
#define VK_INSTANCE_FUNCTION(name)\
	name = (PFN_##name)vkGetInstanceProcAddr(vulkan_instance, (const char *)#name);\
	if (!name) _abort("Failed to load Vulkan function %s.", #name);
#define VK_DEVICE_FUNCTION(name)

#include "vulkan_functions.h"

#undef VK_EXPORTED_FUNCTION
#undef VK_GLOBAL_FUNCTION
#undef VK_INSTANCE_FUNCTION
#undef VK_DEVICE_FUNCTION

	{
		VkDebugUtilsMessengerCreateInfoEXT create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		create_info.pfnUserCallback = vulkan_debug_message_callback;
		VK_CHECK(vkCreateDebugUtilsMessengerEXT(vulkan_instance, &create_info, NULL, &vulkan_debug_messenger));
	}

	Input input = {};

	Execution_State execution_state = RUNNING_STATE;
	while (execution_state != EXITING_STATE) {
		execution_state = handle_platform_events(&input, execution_state);
	}

	cleanup();
}
