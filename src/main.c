#include <libdragon.h>

#define TICKRATE 5
#define TICKDELTA (1.0f / (float)TICKRATE)

#define CELL_SIZE 16
#define MAP_WIDTH_CELLS 18
#define MAP_HEIGHT_CELLS 13

#define SNAKE_CELLS_MAX 0xFF

#define DEATH_SCREEN_CELL_COUNT 30

enum {
	SNAKE_DIR_LEFT,
	SNAKE_DIR_RIGHT,
	SNAKE_DIR_UP,
	SNAKE_DIR_DOWN,
	SNAKE_DIR_COUNT
};

typedef struct {
	int position[2];
} snake_cell_t;

typedef struct {
	snake_cell_t cells[SNAKE_CELLS_MAX];
	int cell_count, dir, is_dead;
} snake_t;

typedef struct {
	int position[2];
} food_t;

static void snake_init(snake_t *s)
{
	const int x = MAP_WIDTH_CELLS >> 1, y = MAP_HEIGHT_CELLS >> 1;
	s->cells->position[0] = x;
	s->cells->position[1] = y;
	s->cell_count = 1;
	s->dir = SNAKE_DIR_RIGHT;
	s->is_dead = 0;
}

static void food_init(food_t *f, const snake_t *s)
{
	int is_inside_snake;
	do {
		is_inside_snake = 0;
		f->position[0] = rand() % MAP_WIDTH_CELLS;
		f->position[1] = rand() % MAP_HEIGHT_CELLS;
		for (int i = 0; i < s->cell_count; i++) {
			is_inside_snake +=
				s->cells[i].position[0] == f->position[0] &&
				s->cells[i].position[1] == f->position[1];
		}
	} while (is_inside_snake);
}

static void map_render(const snake_t *s, const food_t *f)
{
	float snake_cell_colvals[SNAKE_CELLS_MAX];
	for (int i = 0; i < s->cell_count; i++)
		snake_cell_colvals[i] =
			1.0f - (((float)i / (float)s->cell_count) * 0.75f);
	rdpq_set_mode_fill(RGBA32(0, 0, 0, 0));
	for (int y = 0; y < MAP_HEIGHT_CELLS; y++) {
		for (int x = 0; x < MAP_WIDTH_CELLS; x++) {
			const int x0 = CELL_SIZE + x * CELL_SIZE;
			const int y0 = CELL_SIZE + y * CELL_SIZE;
			const int x1 = x0 + CELL_SIZE;
			const int y1 = y0 + CELL_SIZE;
			int snake_cell = -1;
			for (int i = 0; i < s->cell_count; i++)
				if (x == s->cells[i].position[0] &&
				    y == s->cells[i].position[1])
					snake_cell = i;
			const int is_food = x == f->position[0] &&
					    y == f->position[1];
			if (snake_cell > -1) {
				const int v =
					snake_cell_colvals[snake_cell] * 0xFF;
				rdpq_set_fill_color(
					RGBA32(v, v >> 1, v >> 2, 0xFF));
			} else if (is_food) {
				rdpq_set_fill_color(
					RGBA32(0xFF, 0x24, 0x24, 0xFF));
			} else {
				rdpq_set_fill_color(
					RGBA32(0x24, 0x24, 0x24, 0xFF));
			}
			rdpq_fill_rectangle(x0 + 1, y0 + 1, x1 - 1, y1 - 1);
		}
	}

	/* draw death screen */
	if (!s->is_dead)
		return;

	const int death_screen_tile_positions[DEATH_SCREEN_CELL_COUNT][2] = {
		{ 4 + 0, 4 + 0 },  { 4 + 1, 4 + 0 },  { 4 + 0, 4 + 1 },
		{ 4 + 0, 4 + 2 },  { 4 + 0, 4 + 3 },  { 4 + 0, 4 + 4 },
		{ 4 + 1, 4 + 4 },  { 4 + 2, 4 + 1 },  { 4 + 2, 4 + 2 },
		{ 4 + 2, 4 + 3 },  { 4 + 4, 4 + 0 },  { 4 + 4, 4 + 1 },
		{ 4 + 4, 4 + 2 },  { 4 + 4, 4 + 3 },  { 4 + 4, 4 + 4 },
		{ 4 + 5, 4 + 4 },  { 4 + 6, 4 + 4 },  { 4 + 5, 4 + 2 },
		{ 4 + 5, 4 + 0 },  { 4 + 6, 4 + 0 },  { 4 + 8, 4 + 0 },
		{ 4 + 9, 4 + 0 },  { 4 + 8, 4 + 1 },  { 4 + 8, 4 + 2 },
		{ 4 + 8, 4 + 3 },  { 4 + 8, 4 + 4 },  { 4 + 9, 4 + 4 },
		{ 4 + 10, 4 + 1 }, { 4 + 10, 4 + 2 }, { 4 + 10, 4 + 3 },
	};
	rdpq_set_fill_color(RGBA32(0xFF, 0xFF, 0x00, 0xFF));
	for (int i = 0; i < DEATH_SCREEN_CELL_COUNT; i++) {
		const int x0 = CELL_SIZE +
			       death_screen_tile_positions[i][0] * CELL_SIZE;
		const int y0 = CELL_SIZE +
			       death_screen_tile_positions[i][1] * CELL_SIZE;
		const int x1 = x0 + CELL_SIZE;
		const int y1 = y0 + CELL_SIZE;
		rdpq_fill_rectangle(x0 + 1, y0 + 1, x1 - 1, y1 - 1);
	}
}

static void update(snake_t *s, food_t *f)
{
	const float delta_time = display_get_delta_time();
	static float time_accum = 0;
	time_accum += delta_time;
	joypad_poll();
	static joypad_inputs_t input_stored;
	if (time_accum < TICKDELTA) {
		input_stored = joypad_get_inputs(JOYPAD_PORT_1);
		return;
	}
	time_accum -= TICKDELTA;
	joypad_inputs_t input = input_stored;
	if (s->is_dead) {
		if (input.btn.start || input.btn.a || input.btn.b) {
			snake_init(s);
			food_init(f, s);
		}
		return;
	}
	const int move[2] = { (input.btn.d_right | input.btn.c_right) -
				      (input.btn.d_left | input.btn.c_left),
			      (input.btn.d_down | input.btn.c_down) -
				      (input.btn.d_up | input.btn.c_up) };

	/* change direction */
	if (move[0] < 0) {
		if (s->dir != SNAKE_DIR_RIGHT)
			s->dir = SNAKE_DIR_LEFT;
	} else if (move[0] > 0) {
		if (s->dir != SNAKE_DIR_LEFT)
			s->dir = SNAKE_DIR_RIGHT;
	} else if (move[1] < 0) {
		if (s->dir != SNAKE_DIR_DOWN)
			s->dir = SNAKE_DIR_UP;
	} else if (move[1] > 0) {
		if (s->dir != SNAKE_DIR_UP)
			s->dir = SNAKE_DIR_DOWN;
	}
	/* update snake cell positions */
	for (int i = s->cell_count - 1; i > 0; i--) {
		snake_cell_t *cur = s->cells + i, *next = s->cells + i - 1;
		cur->position[0] = next->position[0];
		cur->position[1] = next->position[1];
	}
	s->cells->position[0] +=
		(s->dir == SNAKE_DIR_RIGHT) - (s->dir == SNAKE_DIR_LEFT);
	s->cells->position[1] +=
		(s->dir == SNAKE_DIR_DOWN) - (s->dir == SNAKE_DIR_UP);
	const int is_out_of_bounds =
		s->cells[0].position[0] >= MAP_WIDTH_CELLS ||
		s->cells[0].position[0] < 0 ||
		s->cells[0].position[1] >= MAP_HEIGHT_CELLS ||
		s->cells[0].position[1] < 0;
	int has_hit_body = 0;
	for (int i = 1; i < s->cell_count; i++) {
		has_hit_body +=
			s->cells->position[0] == s->cells[i].position[0] &&
			s->cells->position[1] == s->cells[i].position[1];
	}
	if (is_out_of_bounds || has_hit_body) {
		s->is_dead = 1;
		return;
	}

	/* eating food */
	const int is_eating_food = s->cells->position[0] == f->position[0] &&
				   s->cells->position[1] == f->position[1];
	if (is_eating_food) {
		s->cells[s->cell_count].position[0] = -1;
		s->cells[s->cell_count].position[1] = -1;
		++s->cell_count;
		food_init(f, s);
	}
}

static void render(const snake_t *s, const food_t *f)
{
	rdpq_attach(display_get(), NULL);
	map_render(s, f);
	rdpq_detach_show();
}

int main(void)
{
	srand(TICKS_READ());
	debug_init_isviewer();
	debug_init_usblog();
	asset_init_compression(3);
	dfs_init(DFS_DEFAULT_LOCATION);

	joypad_init();

	display_init(RESOLUTION_320x240, DEPTH_16_BPP, 2, GAMMA_NONE,
		     FILTERS_RESAMPLE);
	rdpq_init();

	snake_t snake;
	food_t food;
	snake_init(&snake);
	food_init(&food, &snake);
	for (;;) {
		update(&snake, &food);
		render(&snake, &food);
	}

	return 0;
}
