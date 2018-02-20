/***************************************************************************************************/
/*          Copyright (C) 2015-2017 By Yang Chen (yngccc@gmail.com). All Rights Reserved.          */
/***************************************************************************************************/

#include "platform_windows.cpp"
#include "math.cpp"
#include "vulkan.cpp"

#define RAPIDJSON_SSE2
#define RAPIDJSON_ASSERT(x) m_assert(x)
#include "../vendor/include/rapidjson/document.h"
#include "../vendor/include/rapidjson/prettywriter.h"
#include "../vendor/include/rapidjson/error/en.h"

#include "assets.cpp"
#include "level.cpp"

struct game {
	bool key_shift;
	bool key_ctrl;
	bool key_alt;
	bool keys_down[512];
	float keys_down_duration[512];
	float keys_down_duration_prev[512];
	vec2 mouse_position;
	bool mouse_down[3];
	float mouse_wheel;

	camera player_camera;

	memory_arena general_memory_arena;
};

bool initialize_game(game *game, vulkan *vulkan) {
	{ // memory
		game->general_memory_arena = {};
		game->general_memory_arena.name = "general";
		game->general_memory_arena.capacity = m_megabytes(64);
		m_assert(allocate_virtual_memory(game->general_memory_arena.capacity, &game->general_memory_arena.memory));
	}
	return true;
}

int WinMain(HINSTANCE instance_handle, HINSTANCE prev_instance_handle, LPSTR cmd_line_str, int cmd_show) {
	set_exe_dir_as_current();
	show_command_prompt();

	struct window window = {};
	m_assert(initialize_window(&window));
	// set_window_fullscreen(&window, true);

	struct vulkan vulkan = {};
	initialize_vulkan(&vulkan, window);

	struct game game = {};
	initialize_game(&game, &vulkan);

	level level = {};
	initialize_level(&level, &vulkan);
	auto extra_level_load = [](rapidjson::Document *json_doc) {};
	level_read_json(&level, &vulkan, "agby_assets\\levels\\level_save.json", extra_level_load);
	game.player_camera = level_get_player_camera(&level, &vulkan, 10, 0.5, 0.5);
	
	LARGE_INTEGER performance_frequency = {};
	QueryPerformanceFrequency(&performance_frequency);
	LARGE_INTEGER performance_counters[2] = {};
	uint64 last_frame_time_microsec = 0;
	double last_frame_time_sec = 0;
	bool program_running = true;

	show_window(&window);

	while (program_running) {
		QueryPerformanceCounter(&performance_counters[0]);

		window.raw_mouse_dx = 0;
		window.raw_mouse_dy = 0;

		while (peek_window_message(&window)) {
			switch (window.msg_type) {
				case window_message_type_destroy:
				case window_message_type_close:
				case window_message_type_quit: {
					program_running = false;
				} break;
				case window_message_type_key_down:
				case window_message_type_key_up: {
					if (window.keycode == keycode_shift) {
						game.key_shift = (window.msg_type == window_message_type_key_down);
					}
					else if (window.keycode == keycode_ctrl) {
						game.key_ctrl = (window.msg_type == window_message_type_key_down);
					}
					else if (window.keycode == keycode_alt) {
						game.key_alt = (window.msg_type == window_message_type_key_down);
					}
					else {
						m_assert(window.keycode < m_countof(game.keys_down));
						game.keys_down[window.keycode] = (window.msg_type == window_message_type_key_down);
					}
				} break;
				case window_message_type_char: {
				} break;
				case window_message_type_mouse_move: {
				} break;
				case window_message_type_mouse_lb_down:
				case window_message_type_mouse_lb_up: {
				} break;
				case window_message_type_mouse_rb_down:
				case window_message_type_mouse_rb_up: {
				} break;
				case window_message_type_mouse_mb_down:
				case window_message_type_mouse_mb_up: {
				} break;
				case window_message_type_mouse_wheel: {
				} break;
			}
		}
		
		vulkan_begin_render(&vulkan);
		level_generate_render_data(&level, &vulkan, game.player_camera, []{});
		level_generate_render_commands(&level, &vulkan, game.player_camera, []{}, []{});
		vulkan_end_render(&vulkan);

		level_entity_component_end_frame(&level);
		level.frame_memory_arena.size = 0;

		QueryPerformanceCounter(&performance_counters[1]);
		last_frame_time_microsec = (performance_counters[1].QuadPart - performance_counters[0].QuadPart) * 1000000 / performance_frequency.QuadPart;
		last_frame_time_sec = (double)last_frame_time_microsec / 1000000;
	}
}

// { // start rendering thread
// 	struct render_thread_data {
// 		struct vulkan *vulkan;
// 		struct level *level;
// 		struct game *game;
// 	};
// 	render_thread_data *render_thread_data = memory_arena_allocate<struct render_thread_data>(&game.general_memory_arena, 1);
// 	*render_thread_data = {&vulkan, &level, &game};
// 	auto render_thread_func = [](void *data) -> DWORD {
// 		auto *render_thread_data = (struct render_thread_data *)data;
// 		struct vulkan *vulkan = render_thread_data->vulkan;
// 		struct level *level = render_thread_data->level;
// 		struct game *game = render_thread_data->game;
// 		while (true) {
// 			vulkan_begin_render(vulkan);
// 			level_generate_render_data(level, vulkan, game->camera, []{});
// 			level_generate_render_commands(level, vulkan, game->camera, []{}, []{});
// 			vulkan_end_render(vulkan, game->screen_shot);
// 		}
// 		return 0;
// 	};
// 	CreateThread(nullptr, 0, render_thread_func, render_thread_data, 0, nullptr);
// }
