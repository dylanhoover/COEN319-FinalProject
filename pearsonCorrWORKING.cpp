#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
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
// map<userid, tuple<map of all movies user rated, sum of rat, count of rat, avg rat>>
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
            if (nreviews > 0)
            {
                // cout << "About to reserve movie"
                movie_list[i]->reserve(movieid, nreviews, float(sum / nreviews));
                copy(custids.begin(), custids.end(), movie_list[i]->cus);
                copy(ratings.begin(), ratings.end(), movie_list[i]->rat);
                copy(dates.begin(), dates.end(), movie_list[i]->dat);
                i++;
                if (i == movieCount)
                {
                    infile.close();
                    return;
                }
            }
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

bool sortdesc_int(const int &a, const int &b)
{
    return (a > b);
}

bool sortdesc_tupleint(const tuple<int, int> &a, const tuple<int, int> &b)
{
    return (get<1>(a) > get<1>(b));
}

bool sortdesc_tuplefloat(const tuple<int, float> &a, const tuple<int, float> &b)
{
    return (get<1>(a) > get<1>(b));
}

int user_analysis()
{
    vector<tuple<int, int>> top_users;
    int user_id;
    int sum = 0;
    for (int i = 0; i < (int)id_list.size(); ++i)
    {
        user_id = id_list[i];
        top_users.push_back(make_tuple(user_id, get<2>(user_data[user_id])));
        sum += get<2>(user_data[user_id]);
    }

    sort(top_users.begin(), top_users.end(), sortdesc_tupleint);
    float average = (float)sum / (float)id_list.size();
    int median = get<1>(top_users[top_users.size() / 2]);

    cout << "Average number of ratings: " << average << endl;
    cout << "Median number of ratings: " << median << endl;
    cout << "Current Users with most ratings:" << endl;
    for (int i = (int)top_users.size(); i >= 0; --i)
    {
        if (get<1>(top_users[i]) == ((int)average) + 10)
        {
            cout << "User " << get<0>(top_users[i]) << " has " << get<1>(top_users[i]) << " ratings." << endl;
            return get<0>(top_users[i]);
        }
    }
    return 0;
}

float inverse_user_frequency(const int movie_id)
{
    int m = 0;
    m = movie_list[movie_id - 1]->nreviews;
    if (m != 0)
    {
        return (log10(200.0 / m));
    }
    else
    {
        return 1.0;
    }
}

void pciuf(const int user_id, const int k_neighbor, vector<tuple<int, float>> &neighbor_list)
{
    float user_avg_rat = get<3>(user_data[user_id]);
    #pragma omp parallel for shared(movie_list, user_data, user_avg_rat) schedule(dynamic)
    for (int i = 0; i < userCount; i++)
    {
        int temp_user_id = id_list[i];
        int user_rat = 0, temp_user_rat = 0, common_movie = 0, movie_id = 0;
        float num = 0.0, d1 = 0.0, d2 = 0.0, dTotal = 0.0;
        #pragma omp parallel for shared(movie_list, user_data, user_avg_rat, num, d1, d2, common_movie, temp_user_id) firstprivate(movie_id, user_rat, temp_user_rat) schedule(dynamic)
        for (int j = 0; j < movieCount; ++j)
        {
            movie_id = movie_list[j]->movieid;
            user_rat = 0;
            temp_user_rat = 0;
            if (get<0>(user_data[user_id]).count(movie_id))
            {
                user_rat = get<0>(user_data[user_id])[movie_id];
            }
            if (get<0>(user_data[temp_user_id]).count(movie_id))
            {
                temp_user_rat = get<0>(user_data[temp_user_id])[movie_id];
            }
            if ((user_rat != 0) && (temp_user_rat != 0))
            {
                float temp_user_avg_rat = get<3>(user_data[temp_user_id]);
                float f1 = float(user_rat) - user_avg_rat;
                float f2 = float(temp_user_rat) - temp_user_avg_rat;
                num += (f1 * f2);
                d1 += pow(f1, 2);
                d2 += pow(f2, 2);
                ++common_movie;
            }
        }
        if ((d1 > 0.0) && (d2 > 0.0))
        {
            dTotal = sqrt(d1) * sqrt(d2);
        }
        if (dTotal != 0.0)
        {
            float similarity = num / dTotal;
            similarity *= (common_movie / (common_movie + 2));
            similarity *= pow(fabs(similarity), 1.5);
            // similarity *= inverse_user_frequency(movie_id);
            neighbor_list.push_back(make_tuple(temp_user_id, similarity));
        }
    }
    sort(neighbor_list.begin(), neighbor_list.end(), sortdesc_tuplefloat);
    if ((int)neighbor_list.size() > k_neighbor)
    {
        neighbor_list.resize(k_neighbor);
    }
}

float calc_pciuf(const int user_id, const int movie_id, const int k_neighbor)
{

    vector<tuple<int, float>> similarity_list;
    similarity_list.erase(similarity_list.begin(), similarity_list.end());
    pciuf(user_id, k_neighbor, similarity_list);

    float user_avg_rat = get<3>(user_data[user_id]);
    float movie_avg_rat = movie_list[movie_id - 1]->mean;
    float num = 0.0, den = 0.0;

#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < (int)similarity_list.size(); ++i)
    {
        int temp_user_id = get<0>(similarity_list[i]);
        int temp_user_sim = get<1>(similarity_list[i]);
        float temp_user_avg_rat = get<3>(user_data[temp_user_id]);
        int temp_user_rat = 0;
        if (get<0>(user_data[temp_user_id]).count(movie_id))
        {
            temp_user_rat = get<0>(user_data[temp_user_id])[movie_id];
        }
        if (temp_user_rat > 0)
        {
            float weight = temp_user_rat - temp_user_avg_rat;
            num += weight * temp_user_sim;
            den += fabs(temp_user_sim);
        }
    }
    if (den != 0)
    {
        return user_avg_rat + num / den;
    }
    else if (movie_avg_rat != 0)
    {
        return movie_avg_rat;
    }
    else
    {
        return user_avg_rat;
    }
}

int main(int argc, char *argv[])
{
    movieMax = 17770;
    movieCount = 300;
    userCount = 0;
    int userCap = 1000;
    movie_list = (movie_t **)malloc((sizeof(movie_t *) * movieCount));
    string file_name = "combined_data_1.txt";
    for (int i = 0; i < movieCount; i++)
    {
        movie_list[i] = new movie_t();
    }
    populate_movie_list(file_name, userCap);
    cout << "Movie list populated" << endl;
    sort(id_list.begin(), id_list.end());
    userCount = id_list.size();
    cout << "Total movie count: " << movieCount << " | Total unique user count: " << userCount << endl;
    cout << endl;

    // user_analysis();
    int testUser = 661903;

    int threads[8] = {1, 2, 4, 8, 12, 16, 24, 32};
    // omp_set_num_threads(1);
    for (int i = 4; i >= 0; --i)
    {
        omp_set_num_threads(threads[i]);
        int movie_id = 30;
        auto start = omp_get_wtime();
        float pcCalc = calc_pciuf(testUser, movie_id, 100);
        auto end = omp_get_wtime();
        double time_spent = (double)(end - start);
        cout << "Movie: " << movie_id << " | PC rating: " << pcCalc << " | Threads: " << threads[i] << " or " << omp_get_num_threads() << " | Done in " << time_spent << endl;
    }

    // srand(time(0));
    // if(testUser == 0) testUser = id_list[1 + (rand() % userCount)];
    // double sum [8]= {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    // vector<tuple<int, int>> reccomendations;
    // double sum = 0.0;
    // vector<int, int> reccomendations;
    // for (int i = 0; i < movieCount; ++i)
    // {
    //     int temp_user_rat = 0;
    //     int movie_id = movie_list[i]->movieid;
    //     int testUser = id_list[1 + (rand() % userCount)];
    //     testUser = 631387;
    //     if (get<0>(user_data[testUser]).count(movie_id))
    //     {
    //         temp_user_rat = get<0>(user_data[testUser])[movie_id];
    //     }
    //     auto start = omp_get_wtime();
    //     float pcCalc = calc_pciuf(testUser, movie_id, 100);
    //     auto end = omp_get_wtime();
    //     double time_spent = (double)(end - start);
    //     sum += time_spent;
    //     cout << endl;
    // }
    // cout << "Done in Avg time of " << setprecision(5) << sum / (double)movieCount << endl;
    // cout << "Done in Avg time of " << setprecision(5) << sum[0] / (double)3 << endl;

    for (int i = 0; i < movieCount; ++i)
    {
        free(movie_list[i]);
    }
    free(movie_list);
}