#include "gltf.h"
#include "../auxVk/auxVk.h"

namespace gltf
{
Model::Model():
	vkglTF::Model(),
	m_animationTimer(0.0f)
{
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
}
