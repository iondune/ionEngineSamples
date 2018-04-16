
float sq(float v)
{
	return v * v;
}

float pow4(float v)
{
	return v * v * v * v;
}

float saturate(float v)
{
	return clamp(v, 0.0, 1.0);
}

const float Pi = 3.14159;

float min3(float a, float b, float c)
{
	return min(a, min(b, c));
}

float rcp(float x)
{
	return 1.0 / x;
}
