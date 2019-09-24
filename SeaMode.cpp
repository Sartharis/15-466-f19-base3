#include "SeaMode.hpp"
#include "DrawLines.hpp"
#include "LitColorTextureProgram.hpp"
#include "Mesh.hpp"
#include "Sprite.hpp"
#include "DrawSprites.hpp"
#include "data_path.hpp"
#include "Sound.hpp"

#include <iostream>

Load< Sound::Sample > sea_noise(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("loopsea.opus"));
});

Load< SpriteAtlas > font_atlas(LoadTagDefault, []() -> SpriteAtlas const * {
	return new SpriteAtlas(data_path("trade-font"));
});

GLuint sea_meshes_for_lit_color_texture_program = 0;
static Load< MeshBuffer > sea_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer *ret = new MeshBuffer(data_path("sea.pnct"));
	sea_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

static Load< Scene > sea_scene(LoadTagLate, []() -> Scene const * {
	Scene *ret = new Scene();
	ret->load(data_path("sea.scene"), [](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		auto &mesh = sea_meshes->lookup(mesh_name);
		scene.drawables.emplace_back(transform);
		Scene::Drawable::Pipeline &pipeline = scene.drawables.back().pipeline;

		pipeline = lit_color_texture_program_pipeline;
		pipeline.vao = sea_meshes_for_lit_color_texture_program;
		pipeline.type = mesh.type;
		pipeline.start = mesh.start;
		pipeline.count = mesh.count;
	});
	return ret;
});

SeaMode::SeaMode() {
	assert(sea_scene->cameras.size() && "Observe requires cameras.");

	for( const Scene::Transform& t : sea_scene->transforms )
	{
		if( t.name == "SubmarineSet")
		{
			submarine_set = const_cast<Scene::Transform*>(&t);
		}
		if( t.name == "Camera" )
		{
			submarine_camera = const_cast<Scene::Transform*>( &t );
			submarine_camera_rot_default = t.rotation;
		}
		if( t.name == "Propeller" )
		{
			propeller = const_cast<Scene::Transform*>( &t );
		}
		printf( "%s \n", t.name.c_str() );
	}

	current_camera = &sea_scene->cameras.front();

	noise_loop = Sound::loop_3D(*sea_noise, 1.0f, glm::vec3(0.0f, 0.0f, 0.0f), 10000.0f);
}

SeaMode::~SeaMode() {
	noise_loop->stop();
}

bool SeaMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) 
{
	return false;
}

void SeaMode::update(float elapsed) {
	noise_angle = std::fmod(noise_angle + elapsed, 2.0f * 3.1415926f);

	//update sound position:
	glm::vec3 center = glm::vec3(10.0f, 4.0f, 1.0f);
	float radius = 10.0f;
	noise_loop->set_position( current_camera->transform->make_local_to_world() * glm::vec4());

	//update listener position:
	glm::mat4 frame = current_camera->transform->make_local_to_world();

	const Uint8 *state = SDL_GetKeyboardState( NULL );

	if( state[SDL_SCANCODE_A] )
	{
		submarine_set->rotation *= glm::quat( glm::vec3( 0.0f, 0.0f, 0.5f*elapsed) );
		submarine_camera_rot = glm::mix( submarine_camera_rot, submarine_camera_rot_max, 0.9f * elapsed );
		
	}
	else if( state[SDL_SCANCODE_D] )
	{
		submarine_set->rotation *= glm::quat( glm::vec3( 0.0f, 0.0f, -0.5f*elapsed ) );
		submarine_camera_rot = glm::mix( submarine_camera_rot, -submarine_camera_rot_max, 0.9f * elapsed );
		
	}
	else
	{
		submarine_camera_rot = glm::mix( submarine_camera_rot, 0.0f, 0.9f * elapsed );
	}
	submarine_camera->rotation = submarine_camera_rot_default * glm::quat( glm::vec3(  0.0f, submarine_camera_rot, 0.0f ) );
	

	if( state[SDL_SCANCODE_W] )
	{
		submarine_speed = glm::mix(submarine_speed, -submarine_speed_max, 0.2f * elapsed );
		propeller->rotation *= glm::quat( glm::vec3( 0.3f, 0.0f, 0.0f) );
	}
	else if( state[SDL_SCANCODE_S] )
	{
		submarine_speed = glm::mix( submarine_speed, submarine_speed_max, 0.2f * elapsed );
		propeller->rotation *= glm::quat( glm::vec3( -0.3f, 0.0f, 0.0f ) );
	}
	else
	{
		submarine_speed = glm::mix( submarine_speed, 0.0f, 0.2f * elapsed );
	}

	if( state[SDL_SCANCODE_Q] )
	{
		submarine_descend = glm::mix( submarine_descend, -submarine_descend_max, 0.2f * elapsed );
	}
	else if( state[SDL_SCANCODE_E] )
	{
		submarine_descend = glm::mix( submarine_descend, submarine_descend_max, 0.2f * elapsed );
	}
	else
	{
		submarine_descend = glm::mix( submarine_descend, 0.0f, 0.2f * elapsed );
	}
	submarine_set->position += ( submarine_set->rotation * glm::vec3( submarine_speed * elapsed, 0.0f, submarine_descend * elapsed ));
	
	//using the sound lock here because I want to update position and right-direction *simultaneously* for the audio code:
	Sound::lock();
	Sound::listener.set_position(frame[3]);
	Sound::listener.set_right(frame[0]);
	Sound::unlock();
}

void SeaMode::draw(glm::uvec2 const &drawable_size) {
	//--- actual drawing ---
	glClearColor(0.0f, 0.0f, 0.1f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	const_cast< Scene::Camera * >(current_camera)->aspect = drawable_size.x / float(drawable_size.y);

	sea_scene->draw(*current_camera);
}
