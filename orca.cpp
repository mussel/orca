#include "orca.h"

using namespace std;

//Declares a Comparator for Neighbours
struct CompareNeighbour{
	bool operator() (Neighbour const &n1, Neighbour const &n2){
		return n1.dist < n2.dist;
	}
};


Outlier::Outlier (Point pt_):
	pt(pt_),
	score (0.0f)
{}

void Outlier::print (){
	cout << "\tOutlier - id:" << pt.id << ";score:" << score << "\n";
}


void Point::print () {
	cout << id << ":";
	cout << "[";
	
	if (attrs.size() > 0)
		cout << attrs [0];
	for (int i = 1; i < attrs.size (); i++)
		cout << "," << attrs [i];
	cout << "]" << endl;
	
}

float Point::calc_distance (Point& pt){
	float sum = 0.0f;
	float tmp;
	int attr_num = pt.attrs.size();

	for (int i = 0; i < attr_num; i++){
		tmp = pt.attrs [i];
		sum += (tmp * tmp);
	}
	return sqrt(sum);
}


void knn (vector<Point>& dataset, Point& target_pt, int k, float dkmin, 
	vector<Neighbour>& neighbours){
	int pt_num = dataset.size ();
	Point tmp_pt;
	Neighbour tmp_nn;
	float dist;
	int curr_neighbour_num = 0;
	//Cmp function
	CompareNeighbour cmp;
	vector<Neighbour>::iterator last_nn;


	neighbours.clear ();
	neighbours.reserve (k);
	//loop through every point in the dataset
	for (int i = 0; i < pt_num; i++) {			
		tmp_pt = dataset [i];
		if (tmp_pt.id == target_pt.id) continue;
		dist = target_pt.calc_distance (tmp_pt);

		if (curr_neighbour_num < k){
			tmp_nn.pt = tmp_pt;
			tmp_nn.dist = dist;
			//simply add the new point as a neighbour
			neighbours.push_back(tmp_nn);
			//maintain the heap state
			std::push_heap(neighbours.begin(), neighbours.end(),cmp);
			curr_neighbour_num++;
		}
		else{
			//check if the new point can be a nn
			if (dist <= neighbours.front ().dist){
				//removes the farthest neighbour
				std::pop_heap(neighbours.begin (), neighbours.end(), cmp);
				//replaces the removed neighbour with the new one
				last_nn = (--neighbours.end());
				(*last_nn).pt = tmp_pt;
				(*last_nn).dist = dist;
				//now, restore the heap state
				std::push_heap (neighbours.begin(), neighbours.end(), cmp);
			}
		}
		//check if we can halt the knn search based on the min threshold
		//dkmin
		if (neighbours.front ().dist < dkmin)
			//the distance to the k-th neighbour is already smaller than dkmin.
			//Since such distance will only continue to decrease, this means
			//that target_pt can not be an outlier. So we can halt the knn
			//search.
			return;
	}
}
