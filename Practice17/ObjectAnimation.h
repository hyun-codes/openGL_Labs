#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <algorithm>
#include <cmath>

struct ObjectState
{
	glm::vec3 position{0.0f};
	glm::mat4 rotation{1.0f};
	float scale = 0.55f;
	int modelIndex = 0;
	glm::vec3 color{1.0f};
};

struct SwapAnimation
{
	bool active = false;
	bool curved = false;
	float elapsed = 0.0f;
	float duration = 2.0f;
	float height = 1.0f;
	glm::vec3 start[2]{};
	glm::vec3 previous[2]{};
};

const float minimumScale = 0.12f;
const float maximumScale = 1.5f;

inline void RotateObject(ObjectState& object, float degrees, const glm::vec3& axis)
{
	// 도형의 로컬 축을 기준으로 자전한다.
	object.rotation = glm::rotate(object.rotation, glm::radians(degrees), axis);
}

inline void OrbitObject(ObjectState& object, float degrees)
{
	glm::mat4 orbit = glm::rotate(glm::mat4(1.0f), glm::radians(degrees), glm::vec3(0.0f, 1.0f, 0.0f));
	object.position = glm::vec3(orbit * glm::vec4(object.position, 1.0f));
	object.rotation = orbit * object.rotation;
}

inline void ScaleObject(ObjectState& object, float factor, bool aroundOrigin)
{
	float nextScale = std::clamp(object.scale * factor, minimumScale, maximumScale);
	if (aroundOrigin) object.position *= nextScale / object.scale;
	object.scale = nextScale;
}

inline void StartSwap(SwapAnimation& animation, const ObjectState (&objects)[2], bool curved)
{
	animation = {};
	animation.active = true;
	animation.curved = curved;
	animation.height = std::max(0.8f, glm::length(objects[1].position - objects[0].position) * 0.5f);
	for (int i = 0; i < 2; ++i)
		animation.start[i] = animation.previous[i] = objects[i].position;
}

inline void UpdateSwap(SwapAnimation& animation, ObjectState (&objects)[2], float deltaTime)
{
	if (!animation.active) return;
	animation.elapsed = std::min(animation.elapsed + deltaTime, animation.duration);
	float progress = animation.elapsed / animation.duration;
	float eased = progress * progress * (3.0f - 2.0f * progress);
	for (int i = 0; i < 2; ++i)
	{
		glm::vec3 path;
		if (animation.curved)
		{
			path = glm::mix(animation.start[i], animation.start[1 - i], eased);
			float direction = i == 0 ? 1.0f : -1.0f;
			path.y += direction * animation.height * std::sin(glm::pi<float>() * eased);
		}
		else
		{
			// 시작 위치가 어디에 있든 반드시 원점을 통과한다.
			path = eased < 0.5f
				? glm::mix(animation.start[i], glm::vec3(0.0f), eased * 2.0f)
				: glm::mix(glm::vec3(0.0f), animation.start[1 - i], (eased - 0.5f) * 2.0f);
		}
		// 이동량을 더하므로 다른 키로 적용한 변환을 덮어쓰지 않는다.
		objects[i].position += path - animation.previous[i];
		animation.previous[i] = path;
	}
	if (progress >= 1.0f) animation.active = false;
}

inline void UpdateCombinedAnimation(ObjectState (&objects)[2], float& scalePhase, float deltaTime)
{
	for (int i = 0; i < 2; ++i)
	{
		RotateObject(objects[i], 60.0f * deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));
		OrbitObject(objects[i], 30.0f * deltaTime);
	}
	float nextPhase = std::fmod(scalePhase + 0.8f * deltaTime, glm::two_pi<float>());
	float factor = std::exp(0.7f * (std::sin(nextPhase) - std::sin(scalePhase)));
	ScaleObject(objects[0], factor, false);
	ScaleObject(objects[1], 1.0f / factor, false);
	scalePhase = nextPhase;
}

inline glm::mat4 MakeObjectTransform(const ObjectState& object, const glm::mat4& baseTransform)
{
	glm::mat4 transform = glm::translate(baseTransform, object.position);
	transform *= object.rotation;
	return glm::scale(transform, glm::vec3(object.scale));
}
