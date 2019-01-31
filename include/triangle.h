#include "graphics_headers.h"

class Triangle {
 public:
  static std::vector<char> ReadFile(const std::string& file_name);
  Triangle(uint32_t width = 800, uint32_t height = 600);

  void Run();

  ~Triangle();

 private:
  struct QueueFamilyIndices {
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;

    bool IsComplete() {
      return graphics_family.has_value() && present_family.has_value();
    }
  };

  struct SwapChainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> present_modes;
  };
  // For choose first GPU with vulkan support found. Can introduce rating
  // system.
  bool CheckDeviceCompatibility(const vk::PhysicalDevice& device,
                                const vk::SurfaceKHR* surface = nullptr);
  Triangle::QueueFamilyIndices FindQueueFamilies(
      const vk::PhysicalDevice& device,
      const vk::SurfaceKHR* surface = nullptr);
  bool CheckDeviceExtensionSupport(const vk::PhysicalDevice& device);

  Triangle::SwapChainSupportDetails QuerySwapChainSupport(
      const vk::PhysicalDevice& device);

  vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(
      const std::vector<vk::SurfaceFormatKHR>& formats);
  vk::PresentModeKHR ChooseSwapPresentMode(
      const std::vector<vk::PresentModeKHR>& present_modes);
  vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);

  vk::ShaderModule CreateShaderModule(const std::vector<char>& code);

  void InitWindow();
  void InitVulkan();
  bool CheckValidationLayerSupport(std::string& failed_layer);
  void CreateInstance();
  void CreateSurface();
  void PickPhysicalDevice();
  void CreateLogicalDevice();
  void CreateSwapChain();
  void CreateImageViews();
  void CreateGraphicsPipeline();
  void MainLoop();

  GLFWwindow* m_window;
  u_int32_t m_window_width, m_window_height;
  vk::PhysicalDevice* m_physical_device = nullptr;
  vk::Device* m_device = nullptr;
  vk::Queue m_graphics_queue, m_present_queue;

  vk::SurfaceKHR m_surface;
  vk::SwapchainKHR m_swap_chain;

  std::vector<vk::Image> m_swap_chain_images;

  struct ChosenSwapChainDetails {
    vk::Format format;
    vk::Extent2D extent;
  } m_swap_chain_details;

  std::vector<vk::ImageView> m_swap_chain_image_views;

#ifdef DEV_MODE
  const bool m_enable_validation_layers = true;
#else
  const bool m_enable_validation_layers = false;
#endif

  const std::vector<const char*> m_validation_layers = {};
  const std::vector<const char*> m_device_extensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  vk::Instance m_instance;
  vk::Semaphore m_image_avaliable, m_render_completed;
  std::vector<vk::Framebuffer> m_swap_chain_buffers;
};