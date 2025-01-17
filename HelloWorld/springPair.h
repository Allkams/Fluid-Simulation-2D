#pragma once
#include "Play.h"

struct SpringPair
{
	uint32_t index1;
	uint32_t index2;
	Vector2f restSpring;

	SpringPair(uint32_t i, uint32_t j, Vector2f f) : index1(i), index2(j), restSpring(f) {};

	// Define equality operator for SpringPair
	bool operator==(const SpringPair& other) const {
		return (index1 == other.index1 && index2 == other.index2) ||
			(index1 == other.index2 && index2 == other.index1);
	}
};

//namespace std {
//	template<>
//	struct hash<SpringPair> {
//		size_t operator()(const SpringPair& pair) const {
//			return hash<uint32_t>()(pair.index1) ^ hash<uint32_t>()(pair.index2) ^ hash<Vector2f>()(pair.restSpring);
//		}
//	};
//}