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

int averageuserRatingInTest(int userId)
{
    //calculate mean of all ratings for a user
    int sum = 0;
    int count = 0;
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

int pearsonCorrelation(int userId, int kNeighbor)
{
    list<int> neightborList;

    int averageRatingInTest = averageuserRatingInTest(userId);
    
    for (int i = 0; i < 17770; i++)
    {
        for (int j = 0; j < movie_list[i]->nreviews; j++)
        {
            if (movie_list[i]->cus[j] != userId)
            {
                //calculate mean of all ratings for a user
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

    return kthNeighbor;
}

int main(int argc, char *argv[])
{
    FILE* fp = fopen("combined_data.txt", "r");
    if (!fp) return 1;

    int ratingsSize = 10, movie_indexSize = 10;
    int* ratings = (int*) malloc(ratingsSize*sizeof(int));
    int* movie_index = (int*) malloc(movie_indexSize*sizeof(int));
    movie_index[0] = 0;
    int ctr = 0, movie_ctr = 0;
    while (!feof(fp))
    {
        char line[1024];
        if(!fgets(line, 1024, fp)) break;

        if (line[strlen(line) - 2]==':') {
            ++movie_ctr;
            if(movie_ctr >= movie_indexSize) {
                movie_indexSize *= 2;
                movie_index = (int*) realloc(movie_index, movie_indexSize*sizeof(int));
            }
            movie_index[movie_ctr] = ctr;
            continue;
        }

        int rating, customer;
        char buf[1024];
        if(sscanf(line, "%d,%d,%s", &customer, &rating, buf) != 3) {
            printf("error parsing line: '%s'\n", line);
        }

        if(ctr >= ratingsSize) {
            ratingsSize *= 2;
            ratings = (int*) realloc(ratings, ratingsSize*sizeof(int));
        }
        ratings[ctr] = rating;
        ++ctr;
    }
    fclose(fp);
    
    // print out the first 10 ratings
    for(int i = 0; i < 10; ++i) {
        cout << ratings[i] << " ";
    }
    cout << endl;
    //print out the first 10 movie indices2
    for(int i = 0; i < 10; ++i) {
        cout << movie_index[i] << " ";
    }
    cout << endl;

    //print ctr and movie_ctr
    cout << ctr << " " << movie_ctr << endl;
    
    free(ratings);
    free(movie_index);
}