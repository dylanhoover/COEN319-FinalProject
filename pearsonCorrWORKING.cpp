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
#include <map>
#include <math.h>
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
        this->movieid = id;
        this->nreviews = nr;
        this->mean = mean;
        cout << "Movie " << id << " reserved: " << this->info() << endl;
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

int movieCount, userCount, movieMax;
movie_t **movie_list;
vector<int> id_list;
map<int, tuple<map<int, int>, int, int, float>> user_data;

void populate_movie_list(const string fileName, const int user_cap)
{
        ifstream infile(fileName);
        string line = "";
        int movieid, nreviews = 0;
        float sum = 0.0;
        int i = 0;
        vector<int> custids;
        vector<int> ratings;
        vector<string> dates;

            while (getline(infile, line))
            {

                    if (line[line.length() - 1] == ':')
                    {
                        // if (nreviews > 0)
                        // {
                        movie_list[i]->reserve(movieid, nreviews, float(sum / nreviews));
                        copy(custids.begin(), custids.end(), movie_list[i]->cus);
                        copy(ratings.begin(), ratings.end(), movie_list[i]->rat);
                        copy(dates.begin(), dates.end(), movie_list[i]->dat);
                        i++;
                        if (i == movieCount + 1)
                        {
                            infile.close();
                            return;
                        }
                        // }
                        movieid = stoi(line.substr(0, line.length() - 1));
                        nreviews = 0;
                        sum = 0.0;
                        custids.erase(custids.begin(), custids.end());
                        ratings.erase(ratings.begin(), ratings.end());
                        dates.erase(dates.begin(), dates.end());
                    }
                    else if (nreviews < user_cap)
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
                        // if (nreviews < user_cap)
                        // {
                        sum += ratings.back();
                        nreviews++;
                        if (count(id_list.begin(), id_list.end(), custids.back()) == 0)
                        {
                            id_list.push_back(custids.back());
                        }
                        get<0>(user_data[custids.back()])[movieid] = ratings.back();
                        get<1>(user_data[custids.back()]) += ratings.back();
                        ++get<2>(user_data[custids.back()]);
                        get<3>(user_data[custids.back()]) = float(get<1>(user_data[custids.back()])) / float(get<2>(user_data[custids.back()]));
                        // }
                        // else
                        // {
                        //     custids.pop_back();
                        //     ratings.pop_back();
                        //     dates.pop_back();
                        // }
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
}

// void populate_user_data()
// {
//     sort(id_list.begin(), id_list.end());
//     // Find the average rating for each user
//     for (int i = 0; i < id_list.size(); ++i)
//     {
//         int user_id = id_list[i];
//         for (int j = 0; j < movieCount; ++j)
//         {
//             for (int k = 0; k < movie_list[j]->nreviews; ++k)
//             {
//                 if (movie_list[j]->cus[k] == user_id)
//                 {
//                     get<0>(user_data[user_id])[movie_list[j]->movieid] = movie_list[j]->rat[k];
//                     get<1>(user_data[user_id]) += movie_list[j]->rat[k];
//                     ++get<2>(user_data[user_id]);
//                 }
//             }
//         }
//         get<3>(user_data[user_id]) = float(get<1>(user_data[user_id])) / float(get<2>(user_data[user_id]));
//     }
// }

float inverse_user_frequency(const int movie_id)
{
    float iuf = 1.0;
    int m = 0;
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < userCount; i++)
    {
        int temp_user_id = id_list[i];
        if (get<0>(user_data[temp_user_id]).count(movie_id))
        {
            ++m;
        }
        if (m != 0)
        {
            iuf = log10(200.0 / m);
        }
    }
    return iuf;
}

void pciuf(const int user_id, const int k_neighbor, vector<tuple<int, float>> &neighbor_list)
{
    #pragma omp parallel
    {
    float user_avg_rat = get<3>(user_data[user_id]);
    #pragma omp for schedule(dynamic)
    for (int i = 0; i < userCount; i++)
    {
        int temp_user_id = id_list[i];
        int common_movie = 0, iuf_mv_id = 0;
        float num = 0.0, d1 = 0.0, d2 = 0.0, dTotal = 0.0;
        for (int j = 0; j < movieCount; ++j)
        {
            iuf_mv_id = movie_list[j]->movieid;
            int user_rat = 0, temp_user_rat = 0;
            if (get<0>(user_data[user_id]).count(iuf_mv_id))
            {
                user_rat = get<0>(user_data[user_id])[iuf_mv_id];
            }
            if (get<0>(user_data[temp_user_id]).count(iuf_mv_id))
            {
                temp_user_rat = get<0>(user_data[temp_user_id])[iuf_mv_id];
            }
            if ((user_rat != 0) && (temp_user_rat != 0))
            {
                float temp_user_avg_rat = get<3>(user_data[temp_user_id]);
                float f1 = float(user_rat) - user_avg_rat;
                float f2 = float(temp_user_rat) - temp_user_avg_rat;
                num += (f1 * f2);
                d1 += pow(f1, 2);
                d2 += pow(f2, 2);
                common_movie += 1;
            }
        }
        if ((d1 > 0) && (d2 > 0))
        {
            dTotal = sqrt(d1) * sqrt(d2);
        }
        if (dTotal != 0.0)
        {
            float similarity = num / dTotal;
            similarity *= (common_movie / (common_movie + 2));
            similarity *= pow(fabs(similarity), 1.5);
            float iuf = inverse_user_frequency(iuf_mv_id);
            similarity *= iuf;
            tuple<int, float> temp_tuple(temp_user_id, similarity);
            neighbor_list.push_back(temp_tuple);
        }
    }
    }
    #pragma omp barrier
    #pragma omp single
    {
    sort(neighbor_list.begin(), neighbor_list.end());
    if (neighbor_list.size() > k_neighbor)
    {
        neighbor_list.resize(k_neighbor);
    }
    }
}

float calc_pciuf(const int user_id, const int movie_id, const int k_neighbor)
{
    
    vector<tuple<int, float>> similarity_list;
    similarity_list.erase(similarity_list.begin(), similarity_list.end());
    pciuf(user_id, k_neighbor, similarity_list);

    float user_avg_rat = get<3>(user_data[user_id]);
    float movie_avg_rat = movie_list[movie_id]->mean;
    float num = 0.0, den = 0.0;

    #pragma omp parallel for schedule(dynamic) reduction(+:num, den)
    for (int i = 0; i < similarity_list.size(); ++i)
    {
        int temp_user_id = get<0>(similarity_list[i]);
        int temp_user_sim = get<1>(similarity_list[i]);
        float temp_user_avg_rat = get<3>(user_data[temp_user_id]);
        int temp_user_rat = 0;
        #pragma omp task shared(temp_user_rat)
        {
            if (get<0>(user_data[temp_user_id]).count(movie_id))
            {
                temp_user_rat = get<0>(user_data[temp_user_id])[movie_id];
            }
            if (temp_user_rat > 0)
            {
                float weight = temp_user_rat - movie_avg_rat;
                num += weight * temp_user_sim;
                den += fabs(temp_user_sim);
            }
        }
    }
    #pragma omp barrier
    float result;
    if (den != 0)
    {
        result = user_avg_rat + num / den;
    }
    else if (movie_avg_rat != 0)
    {
        result = movie_avg_rat;
    }
    else
    {
        result = user_avg_rat;
    }
    return result;
    

    // int resultFinal = (int)(result + 0.5);
    // if (resultFinal > 5)
    // {
    //     resultFinal = 5;
    // }
    // else if (resultFinal <= 0)
    // {
    //     resultFinal = 1;
    // }
    // return resultFinal;
}

int main(int argc, char *argv[])
{
    movieMax = 17770;
    movieCount = 100;
    userCount = 0;
    int userCap = 1000;
    movie_list = (movie_t **)malloc((sizeof(movie_t *) * movieMax));
    omp_set_num_threads(10);
    string file_name = "combined_data_1.txt";

    for (int i = 0; i < movieMax; i++)
    {
        movie_list[i] = new movie_t();
    }
    //do we need to pass each movie data text to a different thread to parallelize that way
    populate_movie_list(file_name, userCap);
    cout << "Movie list populated" << endl;
    cout << endl;
    // populate_user_data();

    sort(id_list.begin(), id_list.end());
    userCount = id_list.size();

    cout << "Total movie count: " << movieCount << " | Total unique user count: " << userCount << endl;
    cout << endl;

    // Choose random test user from id_list
    srand(time(0));
    int testUser = id_list[1 + (rand() % userCount)];
    cout << "Average rating for user " << testUser << " is " << get<3>(user_data[testUser]) << endl;

    // Test PC for random test user on ALL movies
    // for (int i = 0; i < movieCount; ++i)
    // {
    //     int temp_user_rat = 0;
    //     if (get<0>(user_data[testUser]).count(i))
    //     {
    //         temp_user_rat = get<0>(user_data[testUser])[i];
    //     }
    //     cout << "User " << testUser << " original rating for movie " << i << ": " << temp_user_rat << endl;
    //     cout << "User " << testUser << " PC rating for movie " << i << ": " << calc_pciuf(testUser, i, 100) << endl;
    //     cout << endl;
    // }

    // Test PC on one random movie
    int temp_user_rat = 0;
    int randMovieID = 1 + (rand() % movieCount);
    if (get<0>(user_data[testUser]).count(randMovieID))
    {
        temp_user_rat = get<0>(user_data[testUser])[randMovieID];
    }
    cout << "User " << testUser << " original rating for movie " << randMovieID << ": " << temp_user_rat << endl;
    auto start = clock();
    float pcCalc = calc_pciuf(testUser, randMovieID, 100);
    auto end = clock();
    cout << "User " << testUser << " PC rating for movie " << randMovieID << ": " << pcCalc << endl;
    auto time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    cout << "Done in " << time_spent << endl;

    for (int i = 0; i < movieCount; i++)
    {
        free(movie_list[i]);
    }
    free(movie_list);
}