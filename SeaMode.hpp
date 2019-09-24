#pragma once

#include "Mode.hpp"
#include "Scene.hpp"
#include "Sound.hpp"

struct SeaMode : Mode {
	SeaMode();
	virtual ~SeaMode();

	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	Scene::Camera const *current_camera = nullptr;

	Scene::Transform* propeller = nullptr;
	Scene::Transform* submarine_set = nullptr;
	Scene::Transform* submarine_camera = nullptr;
	bool w_down = false;

	glm::quat submarine_camera_rot_default = glm::quat();
	float submarine_camera_rot_max = 0.05f;
	float submarine_camera_rot = 0.0f;

	float submarine_speed_max = 15.0f;
	float submarine_speed = 0.0f;
	float submarine_descend = 0.0f;
	float submarine_descend_max = 10.0f;

	std::shared_ptr< Sound::PlayingSample > noise_loop;
	float noise_angle = 0.0f;
};
