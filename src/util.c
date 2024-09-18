#include <libdragon.h>

#include "config.h"
#include "util.h"

void stick_input_correct(float stick[2], const int normalize)
{
	float stick_mag = sqrtf(stick[0] * stick[0] + stick[1] * stick[1]);
	if (stick_mag < STICK_MAG_MIN) {
		stick[0] = 0.0f;
		stick[1] = 0.0f;
	}
	if (stick_mag > STICK_MAG_MAX) {
		const float diff = STICK_MAG_MAX / stick_mag;
		stick_mag = STICK_MAG_MAX;
		stick[0] *= diff;
		stick[1] *= diff;
	}
	if (normalize) {
		stick[0] /= (float)STICK_MAG_MAX;
		stick[1] /= (float)STICK_MAG_MAX;
		stick_mag /= (float)STICK_MAG_MAX;
	}
}
