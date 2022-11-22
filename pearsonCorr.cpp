#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <cstring>

using namespace std;

typedef struct movie_t
{
    int movieid;  // movie id
    int nreviews; // number of reviews
    float mean;   // mean of all ratings
    int *cus;     // customer ids
    int *rat;     // ratings
    string *dat;  // dates

    movie_t()
    {
        movieid = nreviews = 0;
        mean = 0.0;
        cus = nullptr;
        rat = nullptr;
        dat = nullptr;
    }

    void reserve(const int id, const int nr, const float mean)
    {
        cus = (int *)malloc(nr * sizeof(int));
        rat = (int *)malloc(nr * sizeof(int));
        dat = (string *)malloc(nr * sizeof(string));
        cout << "movie " << id << " reserved" << endl;
        this->movieid = id;
        this->nreviews = nr;
        this->mean = mean;
    }

    string info() const
    {
        return "<" + to_string(movieid) + ", " + to_string(nreviews) + ", " + to_string(mean) + ">";
    }

    ~movie_t()
    {
        if (cus)
        {
            free(cus);
        }
        if (rat)
        {
            free(rat);
        }
        if (dat)
        {
            free(dat);
        }
    }
} movie_t;

movie_t *movie_list[17770];

int main(int argc, char *argv[])
{
    // call constructor for each movie in movie_list
    for (int i = 0; i < 17770; i++)
    {
        movie_list[i] = new movie_t();
    }
    ifstream infile("combined_data.txt");
    string line = "";
    int movieid, nreviews = 0;
    float sum = 0.0;
    int i = 0;
    vector<int> custids;
    vector<int> ratings;
    vector<string> dates;
    cout << "__________O_O__________" << endl;
    while (getline(infile, line))
    {
        // if(i==500) break;
        if (line[line.length() - 1] == ':')
        {
            if (nreviews > 0)
            {
                // cout << "__________" << i << "__________" << endl;
                // cout << movieid << " " << nreviews << " " << float(sum / nreviews) << endl;
                // for (int k = 0; k < nreviews; k++)
                // {
                //     cout << custids[k] << "---" << ratings[k] << "---" << dates[k] << endl;
                // }
                movie_list[i]->reserve(movieid, nreviews, float(sum / nreviews));
                // cout << "__________A__________" << endl;
                copy(custids.begin(), custids.end(), movie_list[i]->cus);
                // cout << "__________B__________" << endl;
                copy(ratings.begin(), ratings.end(), movie_list[i]->rat);
                // cout << "__________C__________" << endl;
                copy(dates.begin(), dates.end(), movie_list[i]->dat);
                // cout << "__________D__________" << endl;
                i++;
            }
            movieid = stoi(line.substr(0, line.length() - 1));
            nreviews = 0;
            sum = 0.0;
            custids.erase(custids.begin(), custids.end());
            ratings.erase(ratings.begin(), ratings.end());
            dates.erase(dates.begin(), dates.end());
        }
        else
        {
            // delimter is comma
            stringstream ss(line);
            string token;
            int j = 0;
            while (getline(ss, token, ','))
            {
                if (j == 0)
                {
                    custids.push_back(stoi(token));
                }
                else if (j == 1)
                {
                    ratings.push_back(stoi(token));
                }
                else
                {
                    dates.push_back(token);
                }
                j++;
            }
            sum += ratings.back();
            nreviews++;
        }
        // if next line is eof, then we need to add the last movie
        if (infile.peek() == EOF)
        {
            movie_list[i]->reserve(movieid, nreviews, float(sum / nreviews));
            memcpy(movie_list[i]->cus, &custids, nreviews * sizeof(int));
            memcpy(movie_list[i]->rat, &ratings, nreviews * sizeof(int));
            memcpy(movie_list[i]->dat, &dates, nreviews * sizeof(string));
        }
    }

    movie_list[123]->info();

    //   if (argc <= 4)
    //   {
    //     cerr << "Invalid options." << endl
    //          << "<program> <A_nrows> <A_ncols> <B_ncols> <fill_factor> [-t <num_threads>]" << endl;
    //     exit(1);
    //   }
    //   int nrows = atoi(argv[1]);
    //   int ncols = atoi(argv[2]);
    //   int ncols2 = atoi(argv[3]);
    //   double factor = atof(argv[4]);
    //   int nthreads = 1;
    //   if (atof(argv[4]) <= 0.0 || atof(argv[4]) >=0.5)
    //   {
    //     cerr << "Invalid Fill Factor. Must be between greater than 0 and less than 0.5." << endl;
    //     exit(1);
    //   }
    //   if (argc == 7 && strcasecmp(argv[5], "-t") == 0)
    //   {
    //     nthreads = atoi(argv[6]);
    //   }
    //   omp_set_num_threads(nthreads);
    //   auto t1 = omp_get_wtime();
    //   //FUNCTION HERE
    //   auto t2 = omp_get_wtime();

    //   FILE *fp = fopen("sparsematmult_time.txt", "a");
    //   fprintf(fp, "Sparse: %u threads, array A: %u x %u, array B: %u x %u, time: %f\n", nthreads, nrows, ncols, ncols, ncols2, (t2 - t1));
    //   cout << "Execution time: " << (t2 - t1) << endl;
}