#pragma once







static inline AlifHashT alif_hashPointerRaw(const void* _ptr) { // 9
	uintptr_t x = (uintptr_t)_ptr;
	x = (x >> 4) | (x << (8 * sizeof(uintptr_t) - 4));
	return (AlifHashT)x;
}







AlifHashT alif_hashBytes(const void*, AlifSizeT); // 24





union AlifHashSecretT { // 45
	/* ensure 24 bytes */
	unsigned char uc[24]{};
	/* two AlifHashT for FNV */
	struct {
		AlifHashT prefix{};
		AlifHashT suffix{};
	} fnv;
	/* two uint64 for SipHash24 */
	struct {
		uint64_t k0{};
		uint64_t k1{};
	} sipHash;
	/* a different (!) AlifHashT for small string optimization */
	struct {
		unsigned char padding[16]{};
		AlifHashT suffix{};
	} djbx33a;
	struct {
		unsigned char padding[16];
		AlifHashT hashSalt{};
	} expat;
};
