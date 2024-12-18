#include "utils.hpp"

/*
	Transforme une valeur `val` pr�sente entre `smin` et `smax` et la retourne
	pr�sente entre `omin` et `omax`
*/
float map(float smin, float smax, float val, float omin, float omax)
{
	float percent = (val - smin) / (smax - smin);
	return (omax - omin) * percent + omin;
}

//int map(int smin, int smax, int val, int omin, int omax)
//{
//	float percent = ((float)(val - smin)) / ((float)(smax - smin));
//	return (omax - omin) * percent + omin;
//}