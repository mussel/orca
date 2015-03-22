#ifndef ORCA_H
#define ORCA_H
#include <iostream>
#include <vector>

struct Point {
	int id;
	std::vector<float> attrs;

	void print ();
} ;
typedef struct Point Point;

struct Outlier{
	public:
		Outlier (Point pt_);
	private:
		Point pt;
		float score;
		std::vector<Point> neighbours;
};
typedef struct Outlier Outlier;	

#endif

