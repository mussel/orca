#include "orca.h"

using namespace std;

Outlier::Outlier (Point pt_):
	pt(pt_),
	score (0.0f)
{}


void Point::print () {
	cout << id << ":";
	cout << "[";
	
	if (attrs.size() > 0)
		cout << attrs [0];
	for (int i = 1; i < attrs.size (); i++)
		cout << "," << attrs [i];
	cout << "]" << endl;
	
}
