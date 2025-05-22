#pragma once

struct Resolution 
{
	int width = 1280;
	int height = 720;

	float AspectRatio() const
	{
		return static_cast<float>(width) / height;
	}
};