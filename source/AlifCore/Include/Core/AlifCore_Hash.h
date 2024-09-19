#pragma once














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
