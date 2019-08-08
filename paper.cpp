#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <iostream>
#include <random>
#include <string>
#include <iterator>
#include <algorithm>
#include <tuple>
#include <random>


//some definitions
#define FALSE 0
#define TRUE 1
#define ADDR long long
#define BOOL char

using namespace std;

typedef struct _MEMACCESS{
    ADDR addr; 
    BOOL is_read; 
} MEMACCESS;

typedef struct _CACHE {
    // some declear
    ADDR tag;
    ADDR lruCount;
    int valid;
} CACHE;

typedef struct _SET {
    int* arr;
    long long lineNumber;
    int replacementPlace;
}MISS_SET;

typedef enum _RPL{LRU=0, RAND=1, GENETIC=2} RPL;

int ways; 
int numSets = 0; 
int entries = 0; 
int shift = 0; 
int set; 
int block; 
ADDR accesses = 0;
ADDR hits = 0;
ADDR misses = 0;
int assoc=8;

CACHE *cache;
vector<MISS_SET> miss_sets;
vector<MISS_SET> copy_miss_sets;


//misc. function
FILE* fp = 0;
char trace_file[100]="memtrace.trc";
BOOL read_new_memaccess(MEMACCESS*);  //read new memory access from the memory trace file (already implemented)
vector<std::pair<MISS_SET, int>> performance_list;
vector<MISS_SET> survivors;
vector<MISS_SET> children ;
vector<MISS_SET> new_generation ;
bool learn_on = false;


using namespace std;

std::string ASCII = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
std::string digits = "0123456789";
std::string ESCAPE = "!@#$%^&*()-_+=`~[]}{,.<>/?";

long long fitness(long long set1, long long set2) {
    // if(set2 - set1 < 0) cout << "ASD : "  << set2 - set1 << endl;
    return (long long)set2 - set1;
}

bool compare_lineNumber(MISS_SET set1, MISS_SET set2) {
    return set1.lineNumber < set2.lineNumber;
}

void print_miss_sets(vector<MISS_SET> miss_sets) {
    for(int i=0; i<miss_sets.size(); i++) {
        cout << "[";
        for(int j=0; j<assoc; j++)
            cout << miss_sets[i].arr[j] << " ";
        cout << "] ";
        if(i != 0)
            cout << fitness(miss_sets[i-1].lineNumber, miss_sets[i].lineNumber);
        cout << endl;
    }
}
void print_miss_one_set(MISS_SET miss_set) {
    cout << "[";
    for(int i=0; i<assoc; i++)
        cout << miss_set.arr[i] << " ";
    cout << "]";
}


// std::string generate_word(int length) {
//     std::string str = ASCII + digits;
//     std::string out;
//     std::sample(str.begin(), str.end(), std::back_inserter(out),
//                 length, std::mt19937{std::random_device{}()});
//     return out;
// } 

// vector<std::string> generate_population(int size, int min_len, int max_len) {
//     vector<std::string> popluation;
//     int length;

//     for(int i=0; i<size; i++) {
//         length = i % (max_len - min_len + 1) + min_len;
//         popluation.push_back(generate_word(length));
//     }
//     return popluation;
// }

// float fitness(std::string password, std::string test_word) {
//     float score = 0.0, len_score = 0.5;

//     if(password.length() != test_word.length()) 
//         return score;

//     score += len_score;

//     for(int i=0; i<password.length(); i++) {
//         if(password[i] == test_word[i])
//             score += 1;
//     }
//     return score / (password.length() + len_score) * 100;
// }

template<template <typename> class P = std::less >
struct compare_pair_second {
    template<class T1, class T2> bool operator()(const std::pair<T1, T2>&left, const std::pair<T1, T2>&right) {
        return P<T2>()(left.second, right.second);
    }
};

vector<std::pair<MISS_SET, int>> compute_performance(vector<MISS_SET> miss_sets) {
    vector<std::pair<MISS_SET, int>> performance_list;
    int score;
    int pred_len;

    for(int i=1; i<miss_sets.size(); i++) {
        if(miss_sets[i-1].lineNumber > miss_sets[i].lineNumber)
            cout << "??? : " << miss_sets[i-1].lineNumber << "    " << miss_sets[i].lineNumber << endl;
        score = fitness(miss_sets[i-1].lineNumber, miss_sets[i].lineNumber);
        performance_list.push_back(std::make_pair(miss_sets[i-1], score));
        // performance_list.push_back(std::make_pair(miss_sets[i], score));
    }
    std::sort(performance_list.begin(), performance_list.end(), compare_pair_second<std::greater>());
    
    return performance_list;
}

// int* generate_set() {
//     std::random_device dev;
//     std::mt19937 rng(dev());
//     std::uniform_int_distribution<std::mt19937::result_type> dist(0, assoc - 1); // distribution in range [0, assoc-1]
//     int* arr = (int*)malloc(sizeof(int) * assoc);
//     for(int i=0; i<assoc; i++)
//         arr[i] = 0;
//     arr[dist(rng)] = 1;
//     return arr;
// }

int Rand(int n) {
    return rand() % n ;
}

vector<MISS_SET> select_survivors(vector<std::pair<MISS_SET, int>> population_sorted, int best_sample, int lucky_few) {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, population_sorted.size() - 1); // distribution in range [0, assoc-1]
    vector<MISS_SET> next_generation;
    vector<std::pair<MISS_SET, int>> lucky_survivors;
    int random_number;
    int upper_bound = 700;

    for(int i=0; i<best_sample; i++) {
        if(population_sorted[i].second >= upper_bound) {
            next_generation.push_back(population_sorted[i].first);
            // cout << "???" << endl;
        }
    }
    
    std::sample(population_sorted.begin(), population_sorted.end(), std::back_inserter(lucky_survivors),
                lucky_few, std::mt19937{std::random_device{}()});

    for(int i=0; i<lucky_few; i++) 
        next_generation.push_back(lucky_survivors[i].first);

    while(next_generation.size() < best_sample + lucky_few) {    
        random_number = dist(rng);
        if(random_number < population_sorted.size() - best_sample) 
            random_number += best_sample; 
        MISS_SET miss_set = population_sorted[random_number].first;
        next_generation.push_back(miss_set);
    }

    std::random_shuffle(next_generation.begin(), next_generation.end(), Rand);
    // print_miss_sets(next_generation);
    return next_generation;
}

MISS_SET create_child(MISS_SET individual1, MISS_SET individual2) {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(1,100); // distribution in range [1, 100]
    MISS_SET child;
    int random_number;
    int diff_replacement;
    long long diff_lineNumber;
    child.arr = (int*)malloc(sizeof(int) * assoc);

    // int min_len_ind = min(individual1.length(), individual2.length());
    
    // diff_replacement = std::abs(individual2.replacementPlace - individual1.replacementPlace);
    diff_lineNumber = std::abs(individual2.lineNumber - individual1.lineNumber);

    diff_replacement = individual1.replacementPlace + individual2.replacementPlace;
    diff_replacement %= assoc;
    // diff_lineNumber = individual2.lineNumber + individual1.lineNumber;
    // diff_lineNumber %= 10000;
    
    for(int i=0; i<assoc; i++) 
        child.arr[i] = 0;
    child.arr[diff_replacement] = 1;
    child.replacementPlace = diff_replacement;
    child.lineNumber = diff_lineNumber;
    
    /*
    for(int i=0; i<assoc; i++) {
        random_number = dist(rng);
        if(random_number < 50) 
            child.arr[i] = individual1.arr[i];
        else 
            child.arr[i] = individual2.arr[i];
    }
    random_number = dist(rng);
    if(random_number < 50) {
        child.lineNumber = individual1.lineNumber;
        child.replacementPlace = individual1.replacementPlace;
    } else {
        child.lineNumber = individual2.lineNumber;
        child.replacementPlace = individual2.replacementPlace;
    }
    */
    // cout << individual1.replacementPlace << endl;
    // cout << diff_replacement << " " << diff_lineNumber << endl;
    
    // cout << individual1.lineNumber << "     " << individual2.lineNumber <<endl;

    return child;
}

vector<MISS_SET> create_children(vector<MISS_SET> parents, int n_child) {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, parents.size() - 1); // distribution in range [0, assoc-1]
    vector<MISS_SET> next_population;
    for(int i=0; i<parents.size()/2; i++) {
        for(int j=0; j<n_child; j++) 
            next_population.push_back(create_child(parents[i], parents[parents.size() - 1 - i]));
    }
    if(parents.size() % 2 != 0) {
        int random_number = dist(rng);
        next_population.push_back(create_child(parents[random_number], parents[random_number - 1]));
    }
    return next_population;
}

MISS_SET mutate(MISS_SET miss_set) {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, assoc - 1); // distribution in range [0, assoc-1]
    std::uniform_int_distribution<std::mt19937::result_type> dist2(1,100); // distribution in range [1,100]
    MISS_SET new_set;
    new_set.arr = (int*)malloc(sizeof(int)*assoc);
    int current_replace_idx;
    int idx = dist(rng);

    for(int i=0; i<assoc; i++)
        new_set.arr[i] = 0;

    new_set.arr[idx] = 1; 
    new_set.replacementPlace = idx;

    int random_number = dist2(rng);
    if(random_number < 50)
        new_set.lineNumber = miss_set.lineNumber + (long long)random_number;
    else 
        new_set.lineNumber = miss_set.lineNumber - (long long)random_number;
    
    return miss_set;
}

vector<MISS_SET> mutate_population(vector<MISS_SET> population, int chance_of_mutation) {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(1, 100); // distribution in range [1, 100]

    for(int i=0; i<population.size(); i++) {
        if(dist(rng) < chance_of_mutation) 
            population[i] = mutate(population[i]);
    }
    return population;
}


///////////////////////////////// CACHE relavant work /////////////////////////////////

//configure the cache
void init_cache(int cache_size, int block_size, int assoc, RPL repl_policy) {
    ways = assoc;
    numSets = (cache_size/block_size)/(assoc); 
    entries = numSets*assoc; 
    set = log2(numSets); 
    block = log2(block_size); 
    shift = set+block; 
    cache = (CACHE*)malloc(entries * sizeof(CACHE)); 

    for(int x=0; x<entries;x++){
        cache[x].tag = -1;
        cache[x].valid = 1;
        cache[x].lruCount = 1;
    }

    srand(time(NULL));
}

//check if the memory access hits on the cache
void isHit(ADDR addr, RPL repl_policy, BOOL isRead, long long lineNumber, int assoc) {
    accesses = accesses+1;
    ADDR addr_tag = addr >> (shift); 
    ADDR addr_set = ((addr - (addr_tag<<(block+set))) >> block);
    
    int under = addr_set*ways; 
    int upper = under+(ways-1); 
    
    int j;
    int hit = 0;
    int random;

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, 100); // distribution in range [0, assoc-1]
    std::uniform_int_distribution<std::mt19937::result_type> prob(1, 100); // distribution in range [0, assoc-1]

    switch(repl_policy) {
        case LRU:
            for(j=under; j<=upper; j++) {
                if(cache[j].tag == addr_tag) {
                    hit = 1; 
                    hits=hits+1;

                    for(int k=under;k<=upper;k++) 
                        cache[k].lruCount++;
                    
                    cache[j].lruCount = 0;
                    break;
                }
            }
            if(hit == 0){
                misses=misses+1;

                int highestAge = 0;
                int highestSpot = 0;
                int m;
                
                for(m=under; m<=upper; m++) {
                    if(cache[m].lruCount>highestAge) {
                        highestAge = cache[m].lruCount; 
                        highestSpot = m; 
                    }
                }

                cache[highestSpot].tag = addr_tag;

                for(m=under; m<=upper; m++)
                    cache[m].lruCount++;
                
                cache[highestSpot].lruCount = 0; 
            } 
        break;
        case RAND:
            random = rand() % ways;  
            
            for(j=under; j<=upper; j++) {
                if(cache[j].tag == addr_tag) {
                    hit = 1; 
                    hits=hits+1;
                 
                    break;
                }
            }
            if(hit == 0){
                misses=misses+1;
                cache[under + random].tag = addr_tag;
            } 
        break;
        case GENETIC:
            for(j=under; j<=upper; j++) {
                if(cache[j].tag == addr_tag) {
                    hit = 1; 
                    hits=hits+1;

                    for(int k=under;k<=upper;k++) 
                        cache[k].lruCount++;
                    
                    cache[j].lruCount = 0;
                    break;
                }
            }
            if(hit == 0){
                misses=misses+1;

                int highestAge = 0;
                int highestSpot = 0;
                int m;
                
                for(m=under; m<=upper; m++) {
                    if(cache[m].lruCount>highestAge) {
                        highestAge = cache[m].lruCount; 
                        highestSpot = m; 
                    }
                }

                if(!learn_on) {
                    cache[highestSpot].tag = addr_tag;
                    for(m=under; m<=upper; m++)
                        cache[m].lruCount++;            
                    cache[highestSpot].lruCount = 0; 
                } else {
                    // cache[performance_list[dist(rng)].first.replacementPlace].tag = addr_tag;
                    // cache[performance_list[0].first.replacementPlace].tag = addr_tag;  // Second BEST!
                    // cache[miss_sets[dist(rng)].replacementPlace].tag = addr_tag;   // BEST!
                    // cache[miss_sets[0].replacementPlace].tag = addr_tag;
                    // cache[copy_miss_sets[dist(rng)].replacementPlace].tag = addr_tag;   
                    for(m=under; m<=upper; m++)
                        cache[m].lruCount++;   

                    int r = dist(rng);
                    if(prob(rng) < 90) {
               
                        cache[performance_list[0].first.replacementPlace].tag = addr_tag;  // Second BEST!
                        cache[performance_list[0].first.replacementPlace].lruCount = 0; 
                       
                        // cache[performance_list[r].first.replacementPlace].tag = addr_tag;  // Second BEST!
                        // cache[performance_list[r].first.replacementPlace].lruCount = 0; 
                    } else {
                        
                        // cache[performance_list[r].first.replacementPlace].tag = addr_tag;  // Second BEST!
                        // cache[performance_list[r].first.replacementPlace].lruCount = 0; 
                        cache[highestSpot].tag = addr_tag;
                        cache[highestSpot].lruCount = 0;                         
                    }
                    
                    /*
                    int r = dist(rng);
                    if(abs(highestSpot - performance_list[r].first.replacementPlace) < 2) {
                        cache[performance_list[r].first.replacementPlace].tag = addr_tag;  // Second BEST!
                        cache[performance_list[r].first.replacementPlace].lruCount = 0; 
                    } else {
                        cache[highestSpot].tag = addr_tag;
                        cache[highestSpot].lruCount = 0; 
                    }
                    */
                    
                    
                }

                int* arr = (int*)malloc(sizeof(int) * assoc);
                for(int k=0; k<assoc; k++)
                    arr[k] = 0;
                arr[upper - highestSpot] = 1;
                MISS_SET set = { arr, lineNumber, upper - highestSpot };
                miss_sets.push_back(set);
            }
        break;
    }
}

//print the simulation statistics
void print_stat(int, int, int, RPL);

//main
int main(int argc, char* argv[])  
{
    int i=0;
    int cache_size=32768; // 32KB
    int block_size=32;
    RPL repl_policy=LRU;
    long long lineNumber = 0;
    int best_sample, lucky_few;
    int n_child = 2;
    int chance_of_mutation = 10;
    int interval = 0;

	/*
    *  Read through command-line arguments for options.
    */
    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == 's') 
                cache_size=atoi(argv[i+1]);
            
            if (argv[i][1] == 'b')
                block_size=atoi(argv[i+1]);
            
            if (argv[i][1] == 'a')
                assoc=atoi(argv[i+1]);
            
            if (argv[i][1] == 'f')
                strcpy(trace_file,argv[i+1]);


            if (argv[i][1] == 'r')
            {
                if(strcmp(argv[i+1],"lru")==0)
                    repl_policy=LRU;
                else if(strcmp(argv[i+1],"rand")==0)
                    repl_policy=RAND;
                else if(strcmp(argv[i+1],"genetic")==0)
                    repl_policy=GENETIC;
                else
                {
                    printf("unsupported replacement policy:%s\n",argv[i+1]);
                    return -1;
                }           
            }
        }
    }
    
    /*
     * main body of cache simulator
    */
    
    init_cache(cache_size, block_size, assoc, repl_policy);   //configure the cache with the cache parameters specified in the input arguments
    while(1)
	{
        MEMACCESS new_access;
        
        BOOL success=read_new_memaccess(&new_access);  //read new memory access from the memory trace file

        if(success!=TRUE)   //check the end of the trace file
            break;

        isHit(new_access.addr, repl_policy, new_access.is_read, lineNumber, assoc);
        if( lineNumber != 0 && (lineNumber % 10000) == 0) interval = 1;
        lineNumber++;

        if(interval) {
            // cout << "SS" << endl;
            for(int i=0; i<30; i++) {
                // print_miss_sets(miss_sets);
                // cout << miss_sets.size() << endl;
                // cout << " "<<miss_sets[55].replacementPlace <<  " " << miss_sets[55].lineNumber << endl;
                
                best_sample = int(miss_sets.size() * 9/10);
                // best_sample = int(miss_sets.size() /2);
                lucky_few = int(miss_sets.size() * 1/10);
                // lucky_few = int(miss_sets.size() /2);


                std::sort(miss_sets.begin(), miss_sets.end(), compare_lineNumber);
                performance_list = compute_performance(miss_sets);
                survivors = select_survivors(performance_list, best_sample, lucky_few);
                // cout << endl << endl;
                // print_miss_sets(survivors);
                // cout << survivors.size() << endl;

                children = create_children(survivors, n_child);
                new_generation = mutate_population(children, chance_of_mutation);
                miss_sets = new_generation;
                // miss_sets = children;
                

                // cout << miss_sets.size() << endl;
                // miss_sets = children;
                // cout << endl << endl;
                // print_miss_sets(new_generation);
                // cout << new_generation.size() << endl;
            }
            // std::copy( miss_sets.begin(), miss_sets.end(), copy_miss_sets.begin() );
            // cout << miss_sets.size() << endl;
            miss_sets.swap(copy_miss_sets);
            std::sort(copy_miss_sets.begin(), copy_miss_sets.end(), compare_lineNumber);
            performance_list = compute_performance(copy_miss_sets);
            miss_sets.clear();
            interval = 0;
            lineNumber = 0;
            learn_on = true;
        }
	}

    // print statistics here
    print_stat(cache_size, block_size, assoc, repl_policy);

    // print_miss_sets(miss_sets);
    // cout << miss_sets.size() << endl;
    // best_sample = int(miss_sets.size() / 2);
    // lucky_few = int(miss_sets.size() / 2);
    // vector<std::pair<MISS_SET, int>> performance_list = compute_performance(miss_sets);
    // vector<MISS_SET> survivors = select_survivors(performance_list, best_sample, lucky_few);
    // cout << endl << endl;
    // print_miss_sets(survivors);
    // cout << survivors.size() << endl;
    // vector<MISS_SET> children = create_children(survivors, n_child);
    // vector<MISS_SET> new_generation = mutate_population(children, chance_of_mutation);
    // cout << endl << endl;
    // print_miss_sets(new_generation);
    // cout << new_generation.size() << endl;

    // cout << endl << endl;
    // for(int i=0; i<performance_list.size(); i++) {
    //     print_miss_one_set(performance_list[i].first);
    //     cout << " " << performance_list[i].second << endl;
    // }
    

	return 0;
}


/*
 * read a new memory access from the memory trace file
 */
BOOL read_new_memaccess(MEMACCESS* mem_access)
{
    ADDR access_addr;
    char access_type[10];
    /*
     * open the mem trace file
     */

    if(fp==NULL)
    {
        fp=fopen(trace_file,"r");
        if(fp==NULL)
        {
            fprintf(stderr,"error opening file");
            exit(2);

        }   
    }

    if(mem_access==NULL)
    {
        fprintf(stderr,"MEMACCESS pointer is null!");
        exit(2);
    }

    if(fscanf(fp,"%llx %s", &access_addr, access_type)!=EOF)
    {
        mem_access->addr=access_addr;
        if(strcmp(access_type,"RD")==0)
            mem_access->is_read=TRUE;
        else
            mem_access->is_read=FALSE;
        
        return TRUE;
    }       
    else
        return FALSE;

}

void print_stat(int cache_size, int block_size, int assoc, RPL repl_policy) {
    /*
    
    execution example)
    g++ paper.cpp && ./a.out -s 32768 -b 32 -a 4 -r lru -f memtrace.trc

    cache_size: 1024 B
    block_size: 32 B
    associativity: 4
    replacement policy : LRU
    cache accesses : ?
    cache_hits : ?
    cache_misses : ?
    cache_miss_rate : ?
    */
    printf("cache_size          : %d\n", cache_size);
    printf("block_size          : %d\n", block_size);
    printf("associativity       : %d\n", assoc);
    printf("replacement policy  : ");
    if(repl_policy == LRU) printf("LRU\n\n");
    else if(repl_policy == GENETIC) printf("GENETIC\n\n");
    else                   printf("RAND\n\n");
    printf("cache accesses  : %lld\n", accesses);
    printf("cache_hits      : %lld\n", hits);
    printf("cache_misses    : %lld\n", misses);
    printf("cache_hit_rate  : %f\n", (float)hits / (float)accesses);
    printf("cache_miss_rate : %f\n\n", (float)misses / (float)accesses);
}
