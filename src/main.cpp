#include <iostream>
#include <vector>
#include <algorithm>
#include <stdint.h>
#include <unistd.h>
#include "orca.h"
//Read file
#include <fstream>
#include <sstream>
#include <string>
#include <climits>

using namespace std;

long long reference_pts_num 	= 100;
int dimension_num 					= 2;
int neighbour_num 					= 5;
int outlier_num						= 10;
char* dataset_filename 				= NULL;
int max_query_num 					= INT_MAX;
int refpt_batch_size				= -1;
int outcand_batch_size 				= -1;
//indicates whether the data will be randomly generated or
//if it will be read from files
bool gen_flag = false;

//***************************************************************************
//* Function forward declaration
//***************************************************************************
void print_knn_search (Point& pt, vector<Neighbour>& neighbours);
//***************************************************************************

typedef struct{
	float val;
	uint id;
}score;


void usage (){

cerr << "Usage:\n" 
	<< "\t-k: # of neighbours\n"
	<< "\t-p: # of outliers to be detected\n"
	<< "\tflag -g: Randomly generated reference and query points\n"
	<< "\t\t-m: # of reference pts to be generated\n"
	<< "\t\t-d: # of dimensions of each pt\n"
	<< "\tno flag: Loads data from filesystem\n"
	<< "\t\t-r: file containing reference points\n";
	exit(1);
}

void load_args (int argc, char** argv){
	//parses the command-line arguments
	opterr = 0;
	char c;
	int aflag = 0;
    char* cvalue = NULL;
	int index;
	
	bool m,d,k,r,p;
	m = false;
	d = false;
	k = false;
	r = false;
	p = false;
    while ((c = getopt (argc, argv, "gm:d:k:r:p:q:b:")) != -1){
    	switch (c){
			case 'g':
				//data will be generated by program
				gen_flag = true;
				break;
			case 'm':
				reference_pts_num = atol(optarg);
				if (reference_pts_num <= 0){
					cerr << "Invalid parameter value. 'm' has to be greater\
					   	than 0" << endl;
			   		exit(1);
				}
				m = true;
				break;
			case 'd':
				dimension_num = atoi(optarg);
				if (dimension_num <= 1){
					cerr << "Invalid parameter value. 'd' has to be greater\
					   	than 1" << endl;
			   		exit(1);
				}
				d = true;
				break;
			case 'k':
				neighbour_num = atoi(optarg);
				if (neighbour_num <= 1 || neighbour_num > 256){
					cerr << "Invalid parameter value. 'k' has to be greater\
					   	than 1 and smaller or equal to 128" << endl;
			   		exit(1);
				}
				k = true;
				break;
			case 'r':
				dataset_filename = optarg;
				r = true;
				break;
			case 'p':
				outlier_num = atoi(optarg);
				if (outlier_num <= 0){
					cerr << "Invalid parameter value. 'p' has to be greater\
					   	than 0" << endl;
					exit(1);
				}
				p = true;
				break;
           case '?':
             if (optopt == 'm' || optopt == 'd' || optopt == 'k' ||  optopt == 'r' ){
				 cerr << "Option -" << (char)optopt << " requires an argument.\n";
			 }				 
             else if (isprint (optopt))
             	cerr << "Unknown option `-" << (char)optopt << "'.\n";
             else
				 cerr << "Unknown option character `\\x" << (char)optopt << "'.\n";
			 usage ();
           default:
             abort ();
           }
	}
	for (index = optind; index < argc; index++)
         cerr << "Non-option argument " <<  argv[index] << "\n";
     
	//check if the parameters options set make sense
	if (gen_flag){
		//if this flag was set, we expect 'm', 'n', 'd'
		if (!m)
			cerr << "# of reference points not set. Using default: m = " << 
				reference_pts_num << endl;
		if (!d)
			cerr << "# dimensions not set. Using default: d = " << 
				dimension_num << endl;
		if (r)
			cerr << "'g' flag set. Ignoring parameter 'r'" << endl;
		 //consistency check
		 if (neighbour_num < 1 || neighbour_num > reference_pts_num){
		     cerr << "Invalid value for k. It must be 1 <= k <= m" << endl;
		     exit(1);
		 }
		cerr << "reference_pts_num:" << reference_pts_num << endl;
		 if (outlier_num <= 0 || outlier_num > reference_pts_num){
		     cerr << "Invalid value for p. It must be 1 <= p <= m" << endl;
		     exit(1);
		}
	}
	else{
		//if g is not set, we load data from files
		if (!r){
			cerr << "File with reference points not specified." << endl;
			usage ();
		}
		if (m || d)
			cerr << "'g' flag NOT set. Ignoring parameters 'm' and 'd'" << endl;
	}

	//check if k was set. If it was not, let the user know the default value will be used
	if (!k)
		cerr << "# of nearest neighbours not set. Using default: k = "
		   << neighbour_num << endl;
	if (!p)
		cerr << "# of outliers to be detected not set. Using default: p = " 
			<< outlier_num << endl;


}

bool readPts(char* file_name, vector<Point>& dataset, long long& pts_num, 
		int& dimension_num)
{
	//The first line of the file should have the following format:
	//<pt_num> <dim_num>
	//where:
	//	pt_num: 	# of points(lines) in the file
	//	dim_num:	# of attributes for each point

	//ifstream dataStream;
	ifstream f_input (file_name);
	int attr_num = 0; //# of attributes of each point
	long long point_num = 0; //# of points in the dataset
	float attr = 0.0f;

	int current_point; // id of the current point been read
	int current_attr; // id of the current attribute been read
	bool read;
	//dataStream.open(file_name, ios::in);
	


	//We read pts stored in the following format
	// pt1: <dim1> <dim2> <dim3> ... <dim'd'>
	// pt2: <dim1> <dim2> <dim3> ... <dim'd'>
	// And store it in memory in a row-major fashion, but in matrix
	//with the following format: mat [dim] [pts]
	// <pt1dim1>		<pt2dim1> 		<pt3dim1> .... 	<pt'M'dim1>
	// <pt1dim2>		<pt2dim2>		<pt3dim2> .... 	<pt'M'dim2>
	// .
	// .
	// <pt1dim'd'>		<pt2dim'd'>		<pt3dim'd'> ...	<pt'M'dim'd'>	
	std::string line;
	//get the first line, to read the # of pts and # attrs 
	if (!getline(f_input, line)){
		//could not get first line. Report error
		cerr << "Couldn't read file: " << file_name << endl;
		exit(1);
	}
	else{
	    std::istringstream iss(line);
		if (!(iss >> point_num >> attr_num)){
			//invalid first line.
			cerr << "First line has invalid format. Expected: \
				<point_num> <attribute_num>" 
				<< endl;
			exit (1);
		}
		pts_num = point_num;
		dimension_num = attr_num;
	}

	dataset.resize (point_num);
	//Now read the dataset
	current_point = 0;
	Point tmp_pt;
	tmp_pt.attrs.resize (dimension_num, 0.0f);
	while (std::getline(f_input, line) && current_point < point_num)
	{
	    std::istringstream iss(line);
		//loop through the attributes parsed
		tmp_pt.id = current_point;
		for (current_attr = 0; current_attr < attr_num; current_attr++){
			//reads current attribute and stores it in: 
			//		-> Line: 	current_attr
			//		-> Column:	current_point
			read = iss >> tmp_pt.attrs [current_attr];
			//check if we read the attribute successfully
			if (!read){
				//there was less attributes than expected in this line. 
				//Report error
				cerr << "File " << file_name << " is corrupted. Expected " << 
					attr_num << " attributes, but only found " << current_attr << 
					". Line: " << current_point + 2 << endl;
				exit(1);
			}
		}
		dataset [current_point] = tmp_pt;
		current_point++;
	}
}

void init_dataset (vector<Point>& ds, int pt_num, int dimension_num){
	Point tmp_pt;
	
	ds.clear ();
	tmp_pt.attrs.resize (dimension_num, 0.0f);
	for (int i = 0; i < pt_num; i++){
		tmp_pt.id = i;
		for (int j = 0; j < dimension_num; j++){
			tmp_pt.attrs [j] = rand () / (float) RAND_MAX;
		}

		//save new pt
		ds.push_back (tmp_pt);
	}
}

int main(int argc, char** argv){
	vector<Point> dataset;
	vector<Outlier> outliers;
	load_args(argc, argv);
	
	if (gen_flag){
		//initilize the matrices A and B with random data
		srand(123456);
		init_dataset (dataset, reference_pts_num, dimension_num);
	}
	else{
		//Read the dataset from a given file
		readPts (dataset_filename, dataset, reference_pts_num, dimension_num);
	}
	orca (dataset, neighbour_num, outlier_num, outliers);

	cout << "outlier_num:" << outliers.size() << endl;
	for (int i = 0; i < outliers.size (); i++){
		outliers [i].print ();
	}

}

void print_knn_search (Point& pt, vector<Neighbour>& neighbours){

	pt.print();
	cerr << "neighbours:" << endl;
	for (int i = 0; i < neighbours.size (); i++){
		cout << "\t" << neighbours [i].pt.id << "(" << neighbours[i].dist << ")"
			<< endl;
	}
	cout << endl;
}

