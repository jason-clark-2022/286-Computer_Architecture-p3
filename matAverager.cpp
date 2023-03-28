#include <thread>
#include <chrono>
#include <fstream>
#include <iostream>
#include <omp.h>
#include <sstream>
#include <string>

using namespace std;
// a class to get more accurate time

class stopwatch
{

private:
	std::chrono::high_resolution_clock::time_point t1;
	std::chrono::high_resolution_clock::time_point t2;
	bool timing;

public:
	stopwatch() : timing(false)
	{
		t1 = std::chrono::high_resolution_clock::time_point::min();
		t2 = std::chrono::high_resolution_clock::time_point::min();
	}

	void start()
	{
		if (!timing)
		{
			timing = true;
			t1 = std::chrono::high_resolution_clock::now();
		}
	}

	void stop()
	{
		if (timing)
		{
			t2 = std::chrono::high_resolution_clock::now();
			timing = false;
		}
	}

	void reset()
	{
		t1 = std::chrono::high_resolution_clock::time_point::min();
		t2 = std::chrono::high_resolution_clock::time_point::min();
		;
		timing = false;
	}

	// will return the elapsed time in seconds as a double
	double getTime()
	{
		std::chrono::duration<double> elapsed = std::chrono::duration_cast<std::chrono::duration<double> >(t2 - t1);
		return elapsed.count();
	}
};

// function takes an data pointer, and the number of rows and cols in the data, and
// allocates and intializes the two dimensional data to a bunch of random numbers

void makeRanddata(unsigned int **&data, unsigned int rows, unsigned int cols, unsigned int seed)
{
	// allocate the data
	data = new unsigned int *[rows];
	for (unsigned int i = 0; i < rows; i++)
	{
		data[i] = new unsigned int[cols];
	}

	// seed the number generator
	// you should change the seed to get different values
	srand(seed);

	// populate the data

	for (unsigned int i = 0; i < rows; i++)
		for (unsigned int j = 0; j < cols; j++)
		{
			data[i][j] = rand() % 10000 + 1; // number between 1 and 10000
		}
}

void getDataFromFile(unsigned int **&data, char fileName[], unsigned int &rows, unsigned int &cols)
{
	ifstream in;
	in.open(fileName);
	if (!in)
	{
		cerr << "error opening file: " << fileName << endl;
		exit(-1);
	}

	in >> rows >> cols;
	data = new unsigned int *[rows];
	for (unsigned int i = 0; i < rows; i++)
	{
		data[i] = new unsigned int[cols];
	}

	// now read in the data

	for (unsigned int i = 0; i < rows; i++)
		for (unsigned int j = 0; j < cols; j++)
		{
			in >> data[i][j];
		}
}

struct highAverage
{
	double highAve;
	int rowsIndex;
	int colsIndex;
};

int main(int argc, char *argv[])
{
	if( argc < 3 )
	{
		cerr<<"Usage: " << argv[0] << " [input data file] [num of threads to use] " << endl;

		cerr<<"or" << endl << "Usage: "<< argv[0] << " rand [num of threads to use] [num rows] [num cols] [seed value]" << endl;
	            exit( 0 );
	}

	// read in the file
	unsigned int rows, cols, seed;
	unsigned int numThreads;
	unsigned int **data;
	// convert numThreads to int
	{
		stringstream ss1;
		ss1 << argv[2];
		ss1 >> numThreads;
	}

	// rows = 3;
	// cols = 3;
	// makeRanddata(data, 3, 3, 5); // data rows cols seed

	string fName(argv[1]);
	if (fName == "rand")
	{
		{
			stringstream ss1;
			ss1 << argv[3];
			ss1 >> rows;
		}
		{
			stringstream ss1;
			ss1 << argv[4];
			ss1 >> cols;
		}
		{
			stringstream ss1;
			ss1 << argv[5];
			ss1 >> seed;
		}
		makeRanddata(data, rows, cols, seed); // data rows cols seed
	}
	else
	{
		getDataFromFile(data, argv[1], rows, cols);
	}

	//UNCOMMENT if you want to print the data data
	// cout << "data: " << endl;
	// for (unsigned int i = 0; i < rows; i++)
	// {
	// 	for (unsigned int j = 0; j < cols; j++)
	// 	{
	// 		cout << "i,j,data " << i << ", " << j << ", ";
	// 		cout << data[i][j] << " ";
	// 	}
	// 	cout << endl;
	// }
	// cout << endl;

	// tell omp how many threads to use
	omp_set_num_threads( numThreads );

	stopwatch S1;
	S1.start();

	/////////////////////////////////////////////////////////////////////
	///////////////////////  YOUR CODE HERE       ///////////////////////
	///////////////         Make it parallel!	 ////////////////////
	/////////////////////////////////////////////////////////////////////

	highAverage ha;
	ha.highAve = 0;

	for (int i = 0; i < rows; i++)
	{ // rows
		for (int j = 0; j < cols; j++)
		{ // cols
			double avg = 0;

			// Regular Case
			if ((i > 0 && i < (rows - 1)) && (j > 0 && j < (cols - 1)))
			{ // Basically if not along the outside border
				avg = (data[i - 1][j - 1] +
					   data[i - 1][j] +
					   data[i - 1][j + 1] +
					   data[i][j - 1] +
					   data[i][j] +
					   data[i][j + 1] +
					   data[i + 1][j - 1] +
					   data[i + 1][j] +
					   data[i + 1][j + 1]) /
					  9;
			}
			// Corner Case 0
			else if (i == 0 && j == 0)
			{
				// May need to check for not out of bounds
				avg = (data[i][j] + data[i][j + 1] + data[i + 1][j] + data[i + 1][j + 1]) / 4;
			}
			// Corner Case 1
			else if (i == 0 && j == (cols - 1))
			{
				avg = (data[i][cols - 1] + data[i][cols - 2] + data[i + 1][cols - 2] + data[i + 1][cols - 1]) / 4;
			}
			// Corner Case 2
			else if (i == (rows - 1) && j == 0)
			{
				avg = (data[rows - 2][0] + data[rows - 2][1] + data[rows - 1][0] + data[rows - 1][1]) / 4;
			}
			// Corner Case 3
			else if (i == (rows - 1) && j == (cols - 1))
			{
				avg = (data[rows - 1][cols - 2] + data[rows - 1][cols - 1] + data[rows - 2][cols - 2] + data[rows - 2][cols - 1]) / 4;
			}
			// Top rows Case
			else if (i == 0 && (j > 0 && j < (cols - 1)))
			{								// IF cell is between the first cell and last cell in rows 0
				avg = (data[i][j - 1] +		// Left of cell
					   data[i][j] +			// current cell
					   data[i][j + 1] +		// Right of cell
					   data[i + 1][j - 1] + // rows 1 first cell
					   data[i + 1][j] +		// rows 1 middle cell
					   data[i + 1][j + 1]) /
					  6; // rows 1 last cell
			}
			// Last rows Case
			else if (i == (rows - 1) && (j > 0 && j < (cols - 1)))
			{								// IF cell is between the first cell and last cell in the last rows
				avg = (data[i][j - 1] +		// Left of cell
					   data[i][j] +			// current cell
					   data[i][j + 1] +		// Right of cell
					   data[i - 1][j - 1] + // rows before last rows, left cell
					   data[i - 1][j] +		// rows before last rows, middle cell
					   data[i - 1][j + 1]) /
					  6; // rows before last rows, right cell
			}
			// First colsumn Case
			else if ((i > 0 && i < (rows - 1)) && j == 0)
			{ // IF cell is first colsumn and not the first or last cell in that colsumn
				avg = (data[i - 1][j] +
					   data[i][j] +			//  x x 0 0 0
					   data[i + 1][j] +		//  x x 0 0 0
					   data[i - 1][j + 1] + //  x x 0 0 0
					   data[i][j + 1] +		//  0 0 0 0 0
					   data[i + 1][j + 1]) /
					  6; //  0 0 0 0 0
			}
			// Last column Case
			else if ((i > 0 && i < (rows - 1)) && j == (cols - 1))
			{ // IF cell is last colsumn and not the first or last cell in that colsumn
				avg = (data[i - 1][j] +
					   data[i][j] +
					   data[i + 1][j] +
					   data[i - 1][j - 1] +
					   data[i][j - 1] +
					   data[i + 1][j - 1]) /
					  6;
			}

			// Compare current average to highest average
			if (avg > ha.highAve)
			{
				ha.highAve = avg;
				ha.rowsIndex = i;
				ha.colsIndex = j;
			}
		}
	}

	S1.stop();

	// print out the max value here
	cout << "Max Value: " << ha.highAve << " M"
		 << "[" << ha.rowsIndex << "][" << ha.colsIndex << "]" << endl;

	cerr << "elapsed time: " << S1.getTime() << endl;
}
