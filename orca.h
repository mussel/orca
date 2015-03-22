#ifndef ORCA_H
#define ORCA_H
#include <iostream>
#include <vector>
#include <math.h>
#include <algorithm>

struct Point {
	int id;
	std::vector<float> attrs;

	void print ();
	float calc_distance (Point& pt);
} ;
typedef struct Point Point;

struct Neighbour {
	Point pt;
	float dist;
};


struct Outlier{
	public:
		Outlier (Point pt_);
		Outlier (){}
		void print();

		Point pt;
		float score;
		std::vector<Neighbour> neighbours;
};
typedef struct Outlier Outlier;	


void knn (std::vector<Point>& dataset, Point& target_pt, int k, float dkmin,
		std::vector<Neighbour>& neighbours);
void orca (std::vector<Point>& dataset, int k, int outlier_num, 
		std::vector<Outlier>& outliers);
#endif

