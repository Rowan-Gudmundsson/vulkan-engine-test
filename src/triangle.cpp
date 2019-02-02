#include "triangle.h"

std::vector<char> Triangle::ReadFile(const std::string& file_name) {
  std::fstream file(file_name, std::ios::in | std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + file_name);
  }

  size_t file_size = (size_t)file.tellg();
  std::vector<char> buffer(file_size);

  file.seekg(0);
  file.read(buffer.data(), file_size);
  file.close();

  return buffer;
}

bool Triangle::CheckDeviceCompatibility(const vk::PhysicalDevice& device,
                                        const vk::SurfaceKHR* surface) {
  vk::PhysicalDeviceProperties device_properties = device.getProperties();
  vk::PhysicalDeviceFeatures device_features = device.getFeatures();

  QueueFamilyIndices indices = FindQueueFamilies(device, surface);

  bool extensions_supported = CheckDeviceExtensionSupport(device);

  bool swap_chain_adequate = false;
  if (extensions_supported) {
    SwapChainSupportDetails swap_chain_support = QuerySwapChainSupport(device);
    swap_chain_adequate = !swap_chain_support.formats.empty() &&
                          !swap_chain_support.present_modes.empty();
  }
  // Can do other stuff later
  return indices.IsComplete() && extensions_supported && swap_chain_adequate;
}

Triangle::QueueFamilyIndices Triangle::FindQueueFamilies(
    const vk::PhysicalDevice& device, const vk::SurfaceKHR* surface) {
  QueueFamilyIndices indices;

  std::vector<vk::QueueFamilyProperties> queue_family_properties =
      device.getQueueFamilyProperties();

  int i = 0;

  for (const auto& queue_family : queue_family_properties) {
    if (queue_family.queueCount > 0 &&
        queue_family.queueFlags & vk::QueueFlagBits::eGraphics) {
      indices.graphics_family = i;
    }

    vk::Bool32 present_support;
    if (surface != nullptr) {
      present_support = device.getSurfaceSupportKHR(i, *surface);
    }
    if (surface != nullptr && queue_family.queueCount > 0 && present_support) {
      indices.present_family = i;
    }

    if (indices.IsComplete()) {
      break;
    }

    i++;
  }
  return indices;
}

Triangle::SwapChainSupportDetails Triangle::QuerySwapChainSupport(
    const vk::PhysicalDevice& device) {
  SwapChainSupportDetails details;

  details.capabilities = device.getSurfaceCapabilitiesKHR(m_surface);
  details.formats = device.getSurfaceFormatsKHR(m_surface);
  details.present_modes = device.getSurfacePresentModesKHR(m_surface);

  return details;
}
bool Triangle::CheckDeviceExtensionSupport(const vk::PhysicalDevice& device) {
  std::vector<vk::ExtensionProperties> available_extensions =
      device.enumerateDeviceExtensionProperties();
  std::set<std::string> required_extensions(m_device_extensions.begin(),
                                            m_device_extensions.end());

  for (const auto& extension : available_extensions) {
    required_extensions.erase(extension.extensionName);
  }
  return required_extensions.empty();
}

vk::SurfaceFormatKHR Triangle::ChooseSwapSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR>& formats) {
  if (formats.size() == 1 && formats[0].format == vk::Format::eUndefined) {
    return {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
  }

  for (const auto& format : formats) {
    if (format.format == vk::Format::eB8G8R8A8Unorm &&
        format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      return format;
    }
  }

  return formats[0];
}

vk::PresentModeKHR Triangle::ChooseSwapPresentMode(
    const std::vector<vk::PresentModeKHR>& present_modes) {
  for (const auto& present_mode : present_modes) {
    if (present_mode == vk::PresentModeKHR::eMailbox) {
      return present_mode;
    }
  }

  return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Triangle::ChooseSwapExtent(
    const vk::SurfaceCapabilitiesKHR& capabilities) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    vk::Extent2D actual_extent = {m_window_width, m_window_height};
    actual_extent.width = std::max(
        capabilities.minImageExtent.width,
        std::min(capabilities.maxImageExtent.width, actual_extent.width));
    actual_extent.height = std::max(
        capabilities.minImageExtent.height,
        std::min(capabilities.maxImageExtent.height, actual_extent.height));

    return actual_extent;
  }
}

vk::ShaderModule Triangle::CreateShaderModule(const std::vector<char>& code) {
  vk::ShaderModuleCreateInfo create_info = {};
  create_info.codeSize = code.size();
  create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

  return m_device->createShaderModule(create_info);
}

Triangle::Triangle(u_int32_t width, uint32_t height) {
  m_window_width = width;
  m_window_height = height;
}

void Triangle::Run() {
  PRINT_STEP("Initialize window", 0)
  InitWindow();
  PRINT_STEP("Initialize vulkan", 0)
  InitVulkan();
  PRINT_STEP("Main loop", 0)
  MainLoop();
}

void Triangle::InitWindow() {
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  m_window = glfwCreateWindow(m_window_width, m_window_height, "Vulkan",
                              nullptr, nullptr);
}

void Triangle::InitVulkan() {
  PRINT_STEP("Create instance", 1)
  CreateInstance();
  PRINT_STEP("Create surface", 1)
  CreateSurface();
  PRINT_STEP("Pick physical device", 1)
  PickPhysicalDevice();
  PRINT_STEP("Create locigal device", 1)
  CreateLogicalDevice();
  PRINT_STEP("Create swapchain", 1)
  CreateSwapChain();
  PRINT_STEP("Create image views", 1)
  CreateImageViews();
  PRINT_STEP("Create render pass", 1)
  CreateRenderPass();
  PRINT_STEP("Create graphics pipeline", 1)
  CreateGraphicsPipeline();
  PRINT_STEP("Create framebuffers", 1)
  CreateFramebuffers();
  PRINT_STEP("Create command pool", 1)
  CreateCommandPool();
  PRINT_STEP("Create command buffers", 1)
  CreateCommandBuffers();
  PRINT_STEP("Create semaphores", 1)
  CreateSemaphores();
}

bool Triangle::CheckValidationLayerSupport(std::string& failed_layer) {
  uint32_t layer_count;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

  std::vector<VkLayerProperties> available_layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

  for (const std::string& layer : m_validation_layers) {
    bool layer_found = false;

    for (const auto& layer_properties : available_layers) {
      if (layer == layer_properties.layerName) {
        layer_found = true;
        break;
      }
    }
    if (!layer_found) {
      failed_layer = layer;
      return false;
    }
  }

  return true;
}

void Triangle::CreateInstance() {
  std::string failed_layer;
  if (m_enable_validation_layers &&
      !CheckValidationLayerSupport(failed_layer)) {
    throw std::runtime_error("Validation layers requested but not supported: " +
                             failed_layer);
  }
  vk::ApplicationInfo app_info = {};

  app_info.pApplicationName = "Hello Triangle";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "No Engine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_MAKE_VERSION(1, 0, 0);

  vk::InstanceCreateInfo create_info = {};
  create_info.pApplicationInfo = &app_info;
  create_info.enabledLayerCount =
      static_cast<uint32_t>(m_validation_layers.size());
  create_info.ppEnabledLayerNames = m_validation_layers.data();

  uint32_t glfw_extension_count = 0;
  const char** glfw_extensions;

  glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

  // std::cout << "GLFW Extensions: " << std::endl;
  // for (int i = 0; i < glfw_extension_count; i++) {
  //   std::cout << "- " << glfw_extensions[i] << std::endl;
  // }

  create_info.enabledExtensionCount = glfw_extension_count;
  create_info.ppEnabledExtensionNames = glfw_extensions;
  create_info.enabledLayerCount = 0;

  vk::Result result = vk::createInstance(&create_info, nullptr, &m_instance);

  if (result != vk::Result::eSuccess) {
    throw std::runtime_error("Failed to create instance.");
  }
}

void Triangle::CreateSurface() {
  VkSurfaceKHR tmp_surface;
  VkResult err =
      glfwCreateWindowSurface(m_instance, m_window, nullptr, &tmp_surface);
  if (err) {
    throw std::runtime_error("Failed to create window surface.");
  }
  m_surface = vk::SurfaceKHR(tmp_surface);
}

void Triangle::PickPhysicalDevice() {
  vk::PhysicalDevice physical_device = {};

  std::vector<vk::PhysicalDevice> devices =
      m_instance.enumeratePhysicalDevices();

  if (devices.size() == 0) {
    throw std::runtime_error("Failed to find GPU with Vulkan support.");
  }

  for (const auto& device : devices) {
    if (CheckDeviceCompatibility(device, &m_surface)) {
      m_physical_device = new vk::PhysicalDevice(device);
      break;
    }
  }

  if (m_physical_device == nullptr) {
    throw std::runtime_error("Failed to find compatible device.");
  }
}

void Triangle::CreateLogicalDevice() {
  QueueFamilyIndices indices =
      FindQueueFamilies(*m_physical_device, &m_surface);

  std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
  std::set<uint32_t> unique_queue_families = {indices.graphics_family.value(),
                                              indices.present_family.value()};

  float queue_priority = 1.0;
  for (uint32_t queue_family : unique_queue_families) {
    vk::DeviceQueueCreateInfo queue_create_info = {};
    queue_create_info.queueFamilyIndex = queue_family;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;
    queue_create_infos.push_back(queue_create_info);
  }

  vk::PhysicalDeviceFeatures device_features = {};

  vk::DeviceCreateInfo device_create_info = {};
  device_create_info.queueCreateInfoCount =
      static_cast<uint32_t>(queue_create_infos.size());
  device_create_info.pQueueCreateInfos = queue_create_infos.data();
  device_create_info.pEnabledFeatures = &device_features;

  device_create_info.enabledExtensionCount =
      static_cast<uint32_t>(m_device_extensions.size());
  device_create_info.ppEnabledExtensionNames = m_device_extensions.data();

  if (m_enable_validation_layers) {
    device_create_info.enabledLayerCount =
        static_cast<uint32_t>(m_validation_layers.size());
    device_create_info.ppEnabledLayerNames = m_validation_layers.data();
  } else {
    device_create_info.enabledLayerCount = 0;
  }

  m_device =
      new vk::Device(m_physical_device->createDevice(device_create_info));

  m_graphics_queue = m_device->getQueue(indices.graphics_family.value(), 0);
  m_present_queue = m_device->getQueue(indices.present_family.value(), 0);
}

void Triangle::CreateSwapChain() {
  SwapChainSupportDetails swap_chain_support =
      QuerySwapChainSupport(*m_physical_device);

  vk::SurfaceFormatKHR surface_format =
      ChooseSwapSurfaceFormat(swap_chain_support.formats);
  vk::PresentModeKHR surface_present_mode =
      ChooseSwapPresentMode(swap_chain_support.present_modes);
  vk::Extent2D surface_extent =
      ChooseSwapExtent(swap_chain_support.capabilities);

  uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;
  if (swap_chain_support.capabilities.maxImageCount > 0 &&
      image_count > swap_chain_support.capabilities.maxImageCount) {
    image_count = swap_chain_support.capabilities.maxImageCount;
  }

  vk::SwapchainCreateInfoKHR swap_chain_create_info = {};
  swap_chain_create_info.surface = m_surface;
  swap_chain_create_info.minImageCount = image_count;
  swap_chain_create_info.imageFormat = surface_format.format;
  swap_chain_create_info.imageColorSpace = surface_format.colorSpace;
  swap_chain_create_info.imageExtent = surface_extent;
  swap_chain_create_info.imageArrayLayers = 1;
  swap_chain_create_info.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

  QueueFamilyIndices indices =
      FindQueueFamilies(*m_physical_device, &m_surface);

  uint32_t queue_family_indices[] = {indices.graphics_family.value(),
                                     indices.present_family.value()};

  if (indices.graphics_family != indices.present_family) {
    swap_chain_create_info.imageSharingMode = vk::SharingMode::eConcurrent;
    swap_chain_create_info.queueFamilyIndexCount = 2;
    swap_chain_create_info.pQueueFamilyIndices = queue_family_indices;
  } else {
    swap_chain_create_info.imageSharingMode = vk::SharingMode::eExclusive;
    swap_chain_create_info.queueFamilyIndexCount = 0;
    swap_chain_create_info.pQueueFamilyIndices = nullptr;
  }

  swap_chain_create_info.preTransform =
      swap_chain_support.capabilities.currentTransform;
  swap_chain_create_info.compositeAlpha =
      vk::CompositeAlphaFlagBitsKHR::eOpaque;
  swap_chain_create_info.presentMode = surface_present_mode;
  swap_chain_create_info.clipped = static_cast<vk::Bool32>(true);
  swap_chain_create_info.oldSwapchain = static_cast<vk::SwapchainKHR>(nullptr);

  m_swap_chain = m_device->createSwapchainKHR(swap_chain_create_info, nullptr);

  m_swap_chain_images = m_device->getSwapchainImagesKHR(m_swap_chain);

  m_swap_chain_details.format = surface_format.format;
  m_swap_chain_details.extent = surface_extent;
}

void Triangle::CreateImageViews() {
  m_swap_chain_image_views.resize(m_swap_chain_images.size());

  for (size_t i = 0; i < m_swap_chain_images.size(); i++) {
    vk::ImageViewCreateInfo create_info = {};
    create_info.image = m_swap_chain_images[i];
    create_info.viewType = vk::ImageViewType::e2D;
    create_info.format = m_swap_chain_details.format;
    create_info.components.r = vk::ComponentSwizzle::eIdentity;
    create_info.components.g = vk::ComponentSwizzle::eIdentity;
    create_info.components.b = vk::ComponentSwizzle::eIdentity;
    create_info.components.a = vk::ComponentSwizzle::eIdentity;
    create_info.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = 1;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = 1;
    m_swap_chain_image_views[i] =
        m_device->createImageView(create_info, nullptr);
  }
}

void Triangle::CreateRenderPass() {
  vk::AttachmentDescription color_attachment = {};
  color_attachment.format = m_swap_chain_details.format;
  color_attachment.samples = vk::SampleCountFlagBits::e1;
  color_attachment.loadOp = vk::AttachmentLoadOp::eClear;
  color_attachment.storeOp = vk::AttachmentStoreOp::eStore;
  color_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
  color_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
  color_attachment.initialLayout = vk::ImageLayout::eUndefined;
  color_attachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

  vk::AttachmentReference color_attachment_ref = {};
  color_attachment_ref.attachment = 0;
  color_attachment_ref.layout = vk::ImageLayout::eColorAttachmentOptimal;

  vk::SubpassDescription subpass = {};
  subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_ref;

  vk::SubpassDependency dependancy = {};
  dependancy.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependancy.dstSubpass = 0;
  dependancy.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
  dependancy.srcAccessMask = vk::AccessFlags(0);
  dependancy.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
  dependancy.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead |
                             vk::AccessFlagBits::eColorAttachmentWrite;

  vk::RenderPassCreateInfo render_pass = {};
  render_pass.attachmentCount = 1;
  render_pass.pAttachments = &color_attachment;
  render_pass.subpassCount = 1;
  render_pass.pSubpasses = &subpass;
  render_pass.dependencyCount = 1;
  render_pass.pDependencies = &dependancy;

  m_render_pass = m_device->createRenderPass(render_pass);
}

void Triangle::CreateGraphicsPipeline() {
  auto vert_shader_code = ReadFile("../shaders/vert.spv");
  auto frag_shader_code = ReadFile("../shaders/frag.spv");

  vk::ShaderModule vert_shader_module = CreateShaderModule(vert_shader_code);
  vk::ShaderModule frag_shader_module = CreateShaderModule(frag_shader_code);

  vk::PipelineShaderStageCreateInfo vert_stage_create_info = {};

  vert_stage_create_info.stage = vk::ShaderStageFlagBits::eVertex;
  vert_stage_create_info.module = vert_shader_module;
  vert_stage_create_info.pName = "main";

  vk::PipelineShaderStageCreateInfo frag_stage_create_info = {};

  frag_stage_create_info.stage = vk::ShaderStageFlagBits::eFragment;
  frag_stage_create_info.module = frag_shader_module;
  frag_stage_create_info.pName = "main";

  vk::PipelineShaderStageCreateInfo shader_stages[] = {vert_stage_create_info,
                                                       frag_stage_create_info};

  vk::PipelineVertexInputStateCreateInfo vertex_input_info = {};
  vertex_input_info.vertexBindingDescriptionCount = 0;
  vertex_input_info.pVertexBindingDescriptions = nullptr;  // Optional
  vertex_input_info.vertexAttributeDescriptionCount = 0;
  vertex_input_info.pVertexAttributeDescriptions = nullptr;

  vk::PipelineInputAssemblyStateCreateInfo input_assembly = {};
  input_assembly.topology = vk::PrimitiveTopology::eTriangleList;
  input_assembly.primitiveRestartEnable = static_cast<vk::Bool32>(false);

  vk::Viewport viewport = {};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = m_swap_chain_details.extent.width;
  viewport.height = m_swap_chain_details.extent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  vk::Rect2D scissor = {};
  scissor.offset = vk::Offset2D{0, 0};
  scissor.extent = m_swap_chain_details.extent;

  vk::PipelineViewportStateCreateInfo viewport_state = {};
  viewport_state.viewportCount = 1;
  viewport_state.pViewports = &viewport;
  viewport_state.scissorCount = 1;
  viewport_state.pScissors = &scissor;

  vk::PipelineRasterizationStateCreateInfo rasterizer = {};
  rasterizer.depthBiasEnable = static_cast<vk::Bool32>(false);
  rasterizer.rasterizerDiscardEnable = static_cast<vk::Bool32>(false);
  rasterizer.polygonMode = vk::PolygonMode::eFill;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = vk::CullModeFlagBits::eBack;
  rasterizer.frontFace = vk::FrontFace::eClockwise;
  rasterizer.depthBiasEnable = static_cast<vk::Bool32>(false);
  rasterizer.depthBiasConstantFactor = 0.0f;  // Optional
  rasterizer.depthBiasClamp = 0.0f;           // Optional
  rasterizer.depthBiasSlopeFactor = 0.0f;     // Optional

  vk::PipelineMultisampleStateCreateInfo multisampling = {};
  multisampling.sampleShadingEnable = static_cast<vk::Bool32>(false);
  multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
  multisampling.minSampleShading = 1.0f;  // Optional
  multisampling.pSampleMask = nullptr;    // Optional
  multisampling.alphaToCoverageEnable =
      static_cast<vk::Bool32>(false);                               // Optional
  multisampling.alphaToOneEnable = static_cast<vk::Bool32>(false);  // Optional

  vk::PipelineColorBlendAttachmentState color_blend_attachment = {};
  color_blend_attachment.colorWriteMask =
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
  color_blend_attachment.blendEnable = static_cast<vk::Bool32>(false);
  color_blend_attachment.srcColorBlendFactor =
      vk::BlendFactor::eOne;  // Optional
  color_blend_attachment.dstColorBlendFactor =
      vk::BlendFactor::eZero;                               // Optional
  color_blend_attachment.colorBlendOp = vk::BlendOp::eAdd;  // Optional
  color_blend_attachment.srcAlphaBlendFactor =
      vk::BlendFactor::eOne;  // Optional
  color_blend_attachment.dstAlphaBlendFactor =
      vk::BlendFactor::eZero;                               // Optional
  color_blend_attachment.alphaBlendOp = vk::BlendOp::eAdd;  // Optional

  vk::PipelineColorBlendStateCreateInfo color_blending = {};
  color_blending.logicOpEnable = static_cast<vk::Bool32>(false);
  color_blending.logicOp = vk::LogicOp::eCopy;  // Optional
  color_blending.attachmentCount = 1;
  color_blending.pAttachments = &color_blend_attachment;
  color_blending.blendConstants[0] = 0.0f;  // Optional
  color_blending.blendConstants[1] = 0.0f;  // Optional
  color_blending.blendConstants[2] = 0.0f;  // Optional
  color_blending.blendConstants[3] = 0.0f;  // Optional

  vk::DynamicState dynamic_states[] = {vk::DynamicState::eViewport,
                                       vk::DynamicState::eLineWidth};

  vk::PipelineDynamicStateCreateInfo dynamic_state = {};
  dynamic_state.dynamicStateCount = 2;
  dynamic_state.pDynamicStates = dynamic_states;

  vk::PipelineLayoutCreateInfo pipeline_layout_info = {};
  pipeline_layout_info.setLayoutCount = 0;             // Optional
  pipeline_layout_info.pSetLayouts = nullptr;          // Optional
  pipeline_layout_info.pushConstantRangeCount = 0;     // Optional
  pipeline_layout_info.pPushConstantRanges = nullptr;  // Optional

  m_pipeline_layout = m_device->createPipelineLayout(pipeline_layout_info);

  vk::GraphicsPipelineCreateInfo pipeline_info = {};
  pipeline_info.stageCount = 2;
  pipeline_info.pStages = shader_stages;
  pipeline_info.pVertexInputState = &vertex_input_info;
  pipeline_info.pInputAssemblyState = &input_assembly;
  pipeline_info.pViewportState = &viewport_state;
  pipeline_info.pRasterizationState = &rasterizer;
  pipeline_info.pMultisampleState = &multisampling;
  pipeline_info.pDepthStencilState = nullptr;
  pipeline_info.pColorBlendState = &color_blending;
  pipeline_info.pDynamicState = nullptr;
  pipeline_info.layout = m_pipeline_layout;
  pipeline_info.renderPass = m_render_pass;
  pipeline_info.subpass = 0;

  m_graphics_pipeline =
      m_device->createGraphicsPipeline(nullptr, pipeline_info);

  m_device->destroyShaderModule(vert_shader_module);
  m_device->destroyShaderModule(frag_shader_module);
}

void Triangle::CreateFramebuffers() {
  m_swap_chain_framebuffers.resize(m_swap_chain_image_views.size());

  for (size_t i = 0; i < m_swap_chain_image_views.size(); i++) {
    vk::ImageView attachments[] = {m_swap_chain_image_views[i]};

    vk::FramebufferCreateInfo create_info = {};
    create_info.renderPass = m_render_pass;
    create_info.attachmentCount = 1;
    create_info.pAttachments = attachments;
    create_info.width = m_swap_chain_details.extent.width;
    create_info.height = m_swap_chain_details.extent.height;
    create_info.layers = 1;

    m_swap_chain_framebuffers.push_back(
        m_device->createFramebuffer(create_info));
  }
}

void Triangle::CreateCommandPool() {
  QueueFamilyIndices queue_family_indices =
      FindQueueFamilies(*m_physical_device, &m_surface);

  vk::CommandPoolCreateInfo pool_info = {};
  pool_info.queueFamilyIndex = queue_family_indices.graphics_family.value();

  m_command_pool = m_device->createCommandPool(pool_info);
}

void Triangle::CreateCommandBuffers() {
  m_command_buffers.resize(m_swap_chain_framebuffers.size());

  vk::CommandBufferAllocateInfo alloc_info = {};
  alloc_info.commandPool = m_command_pool;
  alloc_info.level = vk::CommandBufferLevel::ePrimary;
  alloc_info.commandBufferCount =
      static_cast<uint32_t>(m_command_buffers.size());

  m_command_buffers = m_device->allocateCommandBuffers(alloc_info);

  for (size_t i = 0; i < m_command_buffers.size(); i++) {
    vk::CommandBufferBeginInfo begin_info = {};
    begin_info.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
    begin_info.pInheritanceInfo = nullptr;

    m_command_buffers[i].begin(begin_info);

    vk::RenderPassBeginInfo render_pass_info = {};
    render_pass_info.renderPass = m_render_pass;
    render_pass_info.framebuffer = m_swap_chain_framebuffers[i];
    render_pass_info.renderArea.offset = vk::Offset2D{0, 0};
    render_pass_info.renderArea.extent = m_swap_chain_details.extent;

    std::array<float, 4UL> colors = {0.0f, 0.0f, 0.0f, 1.0f};

    vk::ClearColorValue clear_color = {};
    clear_color.setFloat32(colors);

    vk::ClearValue clear_value = {};
    clear_value.color = clear_color;

    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_value;

    m_command_buffers[i].beginRenderPass(render_pass_info,
                                         vk::SubpassContents::eInline);
    m_command_buffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics,
                                      m_graphics_pipeline);
    m_command_buffers[i].draw(3, 1, 0, 0);
    m_command_buffers[i].endRenderPass();
  }
}

void Triangle::CreateSemaphores() {
  vk::SemaphoreCreateInfo semaphore_info = {};
  m_render_completed = m_device->createSemaphore(semaphore_info);
  m_image_avaliable = m_device->createSemaphore(semaphore_info);
}

void Triangle::MainLoop() {
  while (!glfwWindowShouldClose(m_window)) {
    glfwPollEvents();
    DrawFrame();
  }

  m_device->waitIdle();
}

void Triangle::DrawFrame() {
  uint32_t image_index;
  image_index = m_device
                    ->acquireNextImageKHR(m_swap_chain,
                                          std::numeric_limits<uint64_t>::max(),
                                          m_image_avaliable, nullptr)
                    .value;

  vk::SubmitInfo submit_info = {};

  vk::Semaphore wait_semaphores[] = {m_image_avaliable};
  vk::PipelineStageFlags wait_stages[] = {
      vk::PipelineStageFlagBits::eColorAttachmentOutput};

  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = wait_semaphores;
  submit_info.pWaitDstStageMask = wait_stages;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &m_command_buffers[image_index];

  vk::Semaphore signal_semaphores[] = {m_render_completed};
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = signal_semaphores;

  std::cout << "Getting here." << std::endl;
  m_graphics_queue.submit(submit_info, nullptr);

  vk::PresentInfoKHR present_info = {};
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = signal_semaphores;

  vk::SwapchainKHR swap_chains[] = {m_swap_chain};
  present_info.swapchainCount = 1;
  present_info.pSwapchains = swap_chains;
  present_info.pImageIndices = &image_index;
  present_info.pResults = nullptr;

  m_present_queue.presentKHR(present_info);
}

Triangle::~Triangle() {
  if (m_device != nullptr) {
    for (auto framebuffer : m_swap_chain_framebuffers) {
      m_device->destroyFramebuffer(framebuffer);
    }
    for (auto image_view : m_swap_chain_image_views) {
      m_device->destroyImageView(image_view);
    }
    m_device->destroySwapchainKHR(m_swap_chain);
    m_device->destroyPipeline(m_graphics_pipeline);
    m_device->destroyPipelineLayout(m_pipeline_layout);
    m_device->destroyRenderPass(m_render_pass);
    m_device->destroyCommandPool(m_command_pool);
    m_device->destroySemaphore(m_image_avaliable);
    m_device->destroySemaphore(m_render_completed);
    m_device->destroy();
  }
  m_instance.destroySurfaceKHR(m_surface);
  m_instance.destroy();
  glfwDestroyWindow(m_window);
  glfwTerminate();
}
