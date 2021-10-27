#pragma once

#include <reskinner/Brush.h>

class Brushes
{
public:
	std::unique_ptr<Brush> brush1;
	std::unique_ptr<Brush> brush2;
	std::unique_ptr<Brush> brush3;

	Brushes();
};