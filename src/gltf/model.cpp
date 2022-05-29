#include "gltf.h"
#include "../auxVk/auxVk.h"

namespace gltf
{
using namespace aux;
Model::Model():
	vkglTF::Model(),
	m_animationTimer(0.0f)
{
}

Model::~Model()
{
	destroy(Device::getR());
	for (auto buffer : uniformBuffers) {
		buffer.destroy();
	}
}

void Model::update(int32_t animationIndex, float frameTimer)
{
	if (animations.size() > 0) {
		m_animationTimer += frameTimer;
		if (m_animationTimer > animations[animationIndex].end) {
			m_animationTimer -= animations[animationIndex].end;
		}
		updateAnimation(animationIndex, m_animationTimer);
	}
}

void Model::prepareUniformBuffers()
{
	for (auto& uniformBuffer : uniformBuffers) {
		uniformBuffer.create(Device::getVksDevice(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(shaderValues));
	}
}

void Model::updateShaderValues(Camera& camera)
{
	// Scene
	shaderValues.projection = camera.matrices.perspective;
	shaderValues.view = camera.matrices.view;

	// 此Model Matrix， 只适合Skybox， 不适合其它元素
	shaderValues.model = glm::mat4(glm::mat3(camera.matrices.view));

	shaderValues.camPos = glm::vec3(
		-camera.position.z * sin(glm::radians(camera.rotation.y)) * cos(glm::radians(camera.rotation.x)),
		-camera.position.z * sin(glm::radians(camera.rotation.x)),
		camera.position.z * cos(glm::radians(camera.rotation.y)) * cos(glm::radians(camera.rotation.x))
	);
}

void Model::centerAndScale()
{
	// Center and scale model
	float scale = (1.0f / std::max(aabb[0][0], std::max(aabb[1][1], aabb[2][2]))) * 0.5f;
	glm::vec3 translate = -glm::vec3(aabb[3][0], aabb[3][1], aabb[3][2]);
	translate += -0.5f * glm::vec3(aabb[0][0], aabb[1][1], aabb[2][2]);

	shaderValues.model = glm::mat4(1.0f);
	shaderValues.model[0][0] = scale;
	shaderValues.model[1][1] = scale;
	shaderValues.model[2][2] = scale;
	shaderValues.model = glm::translate(shaderValues.model, translate);
}

void Model::applyShaderValues(uint32_t currentBuffer)
{
	memcpy(uniformBuffers[currentBuffer].mapped, &shaderValues, sizeof(shaderValues));
}

}
