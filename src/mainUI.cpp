#include "v2\v2.h"
#include "auxVk\auxVk.h"

void VulkanExample::updateOverlay()
{
	ImGuiIO& io = ImGui::GetIO();

	ImVec2 lastDisplaySize = io.DisplaySize;
	io.DisplaySize = ImVec2((float)width, (float)height);
	io.DeltaTime = frameTimer;

	io.MousePos = ImVec2(mousePos.x, mousePos.y);
	io.MouseDown[0] = mouseButtons.left;
	io.MouseDown[1] = mouseButtons.right;

	ui->pushConstBlock.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
	ui->pushConstBlock.translate = glm::vec2(-1.0f);

	bool updateShaderParams = false;
	bool updateCBs = false;
	float scale = 1.0f;

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	scale = (float)vks::android::screenDensity / (float)ACONFIGURATION_DENSITY_MEDIUM;
#endif
	ImGui::NewFrame();

	ImGui::SetNextWindowPos(ImVec2(10, 10));
	ImGui::SetNextWindowSize(ImVec2(200 * scale, (sceneModel.animations.size() > 0 ? 440 : 360) * scale), ImGuiSetCond_Always);
	ImGui::Begin("Vulkan glTF 2.0 PBR", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
	ImGui::PushItemWidth(100.0f * scale);

	ui->text("www.saschawillems.de");
	ui->text("%.1d fps (%.2f ms)", lastFPS, (1000.0f / lastFPS));

	if (ui->header("Scene")) {
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
		if (ui->combo("File", selectedScene, scenes)) {
			vkDeviceWaitIdle(device);
			loadScene(scenes[selectedScene]);
			updateDS();
			updateCBs = true;
		}
#else
		if (ui->button("Open gltf file")) {
			std::string filename = "";
#if defined(_WIN32)
			char buffer[MAX_PATH];
			OPENFILENAME ofn;
			ZeroMemory(&buffer, sizeof(buffer));
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.lpstrFilter = "glTF files\0*.gltf;*.glb\0";
			ofn.lpstrFile = buffer;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrTitle = "Select a glTF file to load";
			ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
			if (GetOpenFileNameA(&ofn)) {
				filename = buffer;
			}
#elif defined(__linux__) && !defined(VK_USE_PLATFORM_ANDROID_KHR)
			char buffer[1024];
			FILE* file = popen("zenity --title=\"Select a glTF file to load\" --file-filter=\"glTF files | *.gltf *.glb\" --file-selection", "r");
			if (file) {
				while (fgets(buffer, sizeof(buffer), file)) {
					filename += buffer;
				};
				filename.erase(std::remove(filename.begin(), filename.end(), '\n'), filename.end());
				std::cout << filename << std::endl;
			}
#endif
			if (!filename.empty()) {
				vkDeviceWaitIdle(device);
				loadScene(filename);
				pbr1.updateDS(descriptorPool);
				updateCBs = true;
			}
		}
#endif
		if (ui->combo("Environment", selectedEnvironment, environments)) {
			vkDeviceWaitIdle(device);
			loadEnvironment(environments[selectedEnvironment]);
			pbr1.updateDS(descriptorPool);
			updateCBs = true;
		}
	}

	if (ui->header("Environment")) {
		if (ui->checkbox("Background", &displayBackground)) {
			updateShaderParams = true;
		}
		if (ui->slider("Exposure", &pbr1.shaderParams.exposure, 0.1f, 10.0f)) {
			updateShaderParams = true;
		}
		if (ui->slider("Gamma", &pbr1.shaderParams.gamma, 0.1f, 4.0f)) {
			updateShaderParams = true;
		}
		if (ui->slider("IBL", &pbr1.shaderParams.scaleIBLAmbient, 0.0f, 1.0f)) {
			updateShaderParams = true;
		}
	}

	if (ui->header("Debug view")) {
		const std::vector<std::string> debugNamesInputs = {
			"none", "Base color", "Normal", "Occlusion", "Emissive", "Metallic", "Roughness"
		};
		if (ui->combo("Inputs", &debugViewInputs, debugNamesInputs)) {
			pbr1.shaderParams.debugViewInputs = (float)debugViewInputs;
			updateShaderParams = true;
		}
		const std::vector<std::string> debugNamesEquation = {
			"none", "Diff (l,n)", "F (l,h)", "G (l,v,h)", "D (h)", "Specular"
		};
		if (ui->combo("PBR equation", &debugViewEquation, debugNamesEquation)) {
			pbr1.shaderParams.debugViewEquation = (float)debugViewEquation;
			updateShaderParams = true;
		}
	}

	if (sceneModel.animations.size() > 0) {
		if (ui->header("Animations")) {
			ui->checkbox("Animate", &animate);
			std::vector<std::string> animationNames;
			for (auto animation : sceneModel.animations) {
				animationNames.push_back(animation.name);
			}
			ui->combo("Animation", &animationIndex, animationNames);
		}
	}

	ImGui::PopItemWidth();
	ImGui::End();
	ImGui::Render();

	ImDrawData* imDrawData = ImGui::GetDrawData();

	// Check if ui buffers need to be recreated
	if (imDrawData) {
		VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
		VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

		bool updateBuffers = (ui->vertexBuffer.buffer == VK_NULL_HANDLE) || (ui->vertexBuffer.count != imDrawData->TotalVtxCount) || (ui->indexBuffer.buffer == VK_NULL_HANDLE) || (ui->indexBuffer.count != imDrawData->TotalIdxCount);

		if (updateBuffers) {
			vkDeviceWaitIdle(device);
			if (ui->vertexBuffer.buffer) {
				ui->vertexBuffer.destroy();
			}
			ui->vertexBuffer.create(vulkanDevice, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, vertexBufferSize);
			ui->vertexBuffer.count = imDrawData->TotalVtxCount;
			if (ui->indexBuffer.buffer) {
				ui->indexBuffer.destroy();
			}
			ui->indexBuffer.create(vulkanDevice, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, indexBufferSize);
			ui->indexBuffer.count = imDrawData->TotalIdxCount;
		}

		// Upload data
		ImDrawVert* vtxDst = (ImDrawVert*)ui->vertexBuffer.mapped;
		ImDrawIdx* idxDst = (ImDrawIdx*)ui->indexBuffer.mapped;
		for (int n = 0; n < imDrawData->CmdListsCount; n++) {
			const ImDrawList* cmd_list = imDrawData->CmdLists[n];
			memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
			memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
			vtxDst += cmd_list->VtxBuffer.Size;
			idxDst += cmd_list->IdxBuffer.Size;
		}

		ui->vertexBuffer.flush();
		ui->indexBuffer.flush();

		updateCBs = updateCBs || updateBuffers;
	}

	if (lastDisplaySize.x != io.DisplaySize.x || lastDisplaySize.y != io.DisplaySize.y) {
		updateCBs = true;
	}

	if (updateCBs) {
		vkDeviceWaitIdle(device);
		recordCommandBuffers();
		vkDeviceWaitIdle(device);
	}

	if (updateShaderParams) {
		updateLights();
	}

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	if (mouseButtons.left) {
		mouseButtons.left = false;
	}
#endif
}
