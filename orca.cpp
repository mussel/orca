#include "orca.h"

using namespace std;

//Declares a Comparator for Neighbours
struct CompareNeighbour{
	bool operator() (Neighbour const &n1, Neighbour const &n2){
		return n1.dist < n2.dist;
	}
};

struct CompareOutlier{
	bool operator() (Outlier const &o1, Outlier const &o2){
		return o1.score > o2.score;
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

void orca (vector<Point>& dataset, int k, int outlier_num, vector<Outlier>&
		outliers){

	Outlier out_candidate;
	vector<Outlier>::iterator last_out;
	int pt_num = dataset.size();
	int curr_out_num = 0;
	vector<Neighbour> nn;
	float dkmin = 0.0f;
	Neighbour k_nn;
	CompareOutlier cmp;

	//initialize the outlier heap
	outliers.clear ();
	outliers.reserve (outlier_num);
	for (int i = 0; i < pt_num; i++){
		out_candidate.pt = dataset [i];

		//firstly, we need to find the nn`s of out_candidate
		knn (dataset, out_candidate.pt, k, dkmin, nn);
		//Now we compute the anomaly score of the outlier candidate. In this
		//implementation, the score is equal to the distance to its kth nearest
		//neighbour.
		out_candidate.score = nn.front().dist;

		if (curr_out_num < outlier_num){
			//the outlier list is not full yet. Just add the current outlier
			//candidate
			outliers.push_back (out_candidate);
			std::push_heap (outliers.begin(), outliers.end(), cmp);
			curr_out_num++;
		}
		else{
			//the outlier list is full. We need to check if the current
			//candidate's score is big enough for it to enter the top outlier
			//list
			if (out_candidate.score >= outliers.front().score){
				//copy the neighbours list for this candidate
				out_candidate.neighbours = nn;
				//we pop the candidate with the lowest anomaly score
				std::pop_heap (outliers.begin(), outliers.end(),
						cmp);
				//replace the removed candidate's obj with the new outlier
				//candidate
				last_out = (--outliers.end());
				*last_out = out_candidate;
				std::push_heap (outliers.begin(), outliers.end(),
						cmp);

				//now update the dkmin
				dkmin = outliers.front().score;
			}
		}
	}

	//before returning, sort the list of outlier in descending order
	sort (outliers.begin(), outliers.end(), cmp);
}
