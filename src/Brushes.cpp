#include <reskinner/Brushes.h>

Brushes::Brushes() :
	brush1(std::make_unique<Brush>(Brush("b1", 2.0f))),
	brush2(std::make_unique<Brush>(Brush("b2", 5.0f, 0.1f, 0.8f, true))),
	brush3(std::make_unique<Brush>(Brush("b3", 1.0f, 0.8f, 0.1f))) {}