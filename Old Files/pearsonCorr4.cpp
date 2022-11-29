#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <omp.h>
#include <stdlib.h> /* srand, rand */
#include <time.h>   /* time */
#include <cstring>  /* strcasecmp */
#include <cstdint>
#include <assert.h>
#include <vector>    // std::vector
#include <algorithm> // std::random_shuffle
#include <random>
#include <stdexcept>
#include <list>
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
        cus = (int *)calloc(nr, sizeof(int));
        if (!cus)
        {
            throw std::runtime_error("Could not allocate cus array.");
        }
        rat = (int *)calloc(nr, sizeof(int));
        if (!rat)
        {
            throw std::runtime_error("Could not allocate rat array.");
        }
        dat = (string *)calloc(nr, sizeof(string));
        if (!dat)
        {
            throw std::runtime_error("Could not allocate dat array.");
        }
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

movie_t **movie_list = (movie_t **)malloc((sizeof(movie_t*) * 17770));

float averageuserRatingInTest(int userId)
{
    // calculate mean of all ratings for a user

    // what we need here is to read in a test file
    float sum = 0;
    float count = 0;
    for (int i = 0; i < 17770; i++)
    {
        for (int j = 0; j < movie_list[i]->nreviews; j++)
        {
            if (movie_list[i]->cus[j] == userId)
            {
                sum += movie_list[i]->rat[j];
                count++;
            }
        }
    }

    return sum / count;
}

//calculate average raiting for each movie in training matrix
vector<int> averageRatingFromTraining()
{
    vector<int> result;
    for (int i = 0; i < 17770; i++)
    {
        float sum = 0;
        for (int j = 0; j < movie_list[i]->nreviews; j++)
        {
            sum += movie_list[i]->rat[j];
        }
        result.push_back(sum / movie_list[i]->nreviews);
    }
    return result;
}


list<int> pearsonCorrelation(int userId, int kNeighbor)
{
    list<int> neightborList;

    int averageRatingInTest = averageuserRatingInTest(userId);

    for (int i = 0; i < 17770; i++)
    {
        for (int j = 0; j < movie_list[i]->nreviews; j++)
        {
            if (movie_list[i]->cus[j] != userId)
            {
                // calculate mean of all ratings for a user
                int sum = 0;
                int count = 0;
                for (int i = 0; i < 17770; i++)
                {
                    for (int j = 0; j < movie_list[i]->nreviews; j++)
                    {
                        if (movie_list[i]->cus[j] == movie_list[i]->cus[j])
                        {
                            sum += movie_list[i]->rat[j];
                            count++;
                        }
                    }
                }

                int averageRating = sum / count;

                int numerator = 0;
                int denominator1 = 0;
                int denominator2 = 0;

                for (int i = 0; i < 17770; i++)
                {
                    for (int j = 0; j < movie_list[i]->nreviews; j++)
                    {
                        if (movie_list[i]->cus[j] == movie_list[i]->cus[j])
                        {
                            numerator += (movie_list[i]->rat[j] - averageRating) * (movie_list[i]->rat[j] - averageRatingInTest);
                            denominator1 += (movie_list[i]->rat[j] - averageRating) * (movie_list[i]->rat[j] - averageRating);
                            denominator2 += (movie_list[i]->rat[j] - averageRatingInTest) * (movie_list[i]->rat[j] - averageRatingInTest);
                        }
                    }
                }

                float pearsonCorrelation = numerator / (sqrt(denominator1) * sqrt(denominator2));
                neightborList.push_back(pearsonCorrelation);
            }
        }
    }

    neightborList.sort();

    int kthNeighbor = 0;
    for (int i = 0; i < kNeighbor; i++)
    {
        kthNeighbor = neightborList.back();
        neightborList.pop_back();
    }

    return neightborList;
}

int cal_rating_pearson(int userId, int movieId, int kNeighbor)
{
    int rating = 0;

    int averageRatingInTest = averageuserRatingInTest(userId);
    vector<int> average_RatingFromTraining = averageRatingFromTraining();

    //calculate pearson correlation
    list<int> pearson_Correlation = pearsonCorrelation(userId, kNeighbor);

    int kthNeighbor = 0;
    for (int i = 0; i < kNeighbor; i++)
    {
        kthNeighbor = pearson_Correlation.back();
        pearson_Correlation.pop_back();
    }

    //calculate rating
    float numerator = 0;
    float denominator = 0;
    for (int i = 0; i < 17770; i++)
    {
        for (int j = 0; j < movie_list[i]->nreviews; j++)
        {
            if (movie_list[i]->cus[j] == movie_list[i]->cus[j])
            {
                numerator += (movie_list[i]->rat[j] - average_RatingFromTraining[i]) * kthNeighbor;
                denominator += kthNeighbor;
            }
        }
    }

    rating = averageRatingInTest + (numerator / denominator);

    return rating;
}


int main(int argc, char *argv[])
{

    int kNeighbor = 100;
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
    // omp_set_num_threads(1);
    while (getline(infile, line))
    {
        // if (i == 2000)
        //     break;
        if (line[line.length() - 1] == ':')
        {
            if (nreviews > 0)
            {
                // cout << "__________1__________" << endl;
                movie_list[i]->reserve(movieid, nreviews, float(sum / nreviews));
                // cout << "__________2__________" << endl;
                copy(custids.begin(), custids.end(), movie_list[i]->cus);
                // cout << "__________3__________" << endl;
                copy(ratings.begin(), ratings.end(), movie_list[i]->rat);
                // cout << "__________4__________" << endl;
                copy(dates.begin(), dates.end(), movie_list[i]->dat);
                // cout << "__________5__________" << endl;
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
        if (infile.peek() == EOF)
        {
            movie_list[i]->reserve(movieid, nreviews, float(sum / nreviews));
            copy(custids.begin(), custids.end(), movie_list[i]->cus);
            copy(ratings.begin(), ratings.end(), movie_list[i]->rat);
            copy(dates.begin(), dates.end(), movie_list[i]->dat);
        }
    }
    infile.close();
    // for (int i = 0; i < 2000; i++)
    // {
    //     cout << movie_list[i]->info() << endl;
    // }
    // cout << movie_list[1857]->rat[0] << endl;

    int rating_pearson = cal_rating_pearson(1, 1, kNeighbor);
    cout << rating_pearson << endl;
    for (int i = 0; i < 17770; i++)
    {
        // cout << movie_list[i]->info() << endl;
        free(movie_list[i]);
    }
    free(movie_list);
}