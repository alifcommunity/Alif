#pragma once





enum { // 22
	Alif_Park_Ok = 0,
	Alif_Park_Again = -1,
	Alif_Park_Timeout = -2,
	Alif_Park_Intr = -3,
};








AlifIntT alifParkingLot_park(const void*, const void*, AlifUSizeT, AlifTimeT, void*, AlifIntT); // 61
