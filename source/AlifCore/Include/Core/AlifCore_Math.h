#pragma once


static inline void _alif_adjustErange1(double _x) { // 33
	if (errno == 0) {
		if (_x == ALIF_HUGE_VAL or _x == -ALIF_HUGE_VAL) {
			errno = ERANGE;
		}
	}
	else if (errno == ERANGE and _x == 0.0) {
		errno = 0;
	}
}
