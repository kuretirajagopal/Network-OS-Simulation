#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <bits/stdc++.h>

using namespace std;

#define THREADS 20 

pthread_mutex_t bridge_lock ;

void *north_people(void* n)
{
    int sleep_time = rand() % 5;
    int thread_no = pthread_self();

    pthread_mutex_lock(&bridge_lock);
	cout << "North traveller: " << thread_no << " is travelling on the bridge" << endl;

   	sleep(sleep_time);
	cout << "North traveller: " << thread_no << " completed travelling" << endl;

	pthread_mutex_unlock(&bridge_lock);

	pthread_exit(NULL); 
}

void *south_people(void* n)
{
    int sleep_time = rand() % 5;
    int thread_no = pthread_self();

    pthread_mutex_lock(&bridge_lock);
	cout << "South traveller: " << thread_no << " is travelling on the bridge" << endl;

   	sleep(sleep_time);
	cout << "South traveller: " << thread_no << " completed travelling" << endl;

	pthread_mutex_unlock(&bridge_lock);

	pthread_exit(NULL); 
}

int main()
{
    srand(time(NULL));

    pthread_t north_threads[THREADS];
    pthread_t south_threads[THREADS];

    int temp1, temp2;

    pthread_mutex_init(&bridge_lock, NULL);

    for(int i=0; i<THREADS; i++)
    {
        temp1 = pthread_create(&north_threads[i], NULL, north_people, NULL);
        temp2 = pthread_create(&south_threads[i], NULL, south_people, NULL);

        if(temp1 != 0 || temp2 != 0)
        {
            cout << "The thread creation is unsuccessful" << endl;
            return 1;
        }
    }

    for(int i=0; i<THREADS; i++)
    {
        temp1 = pthread_join(north_threads[i], NULL);
        temp2 = pthread_join(south_threads[i], NULL);

        if(temp1 != 0 || temp2 != 0)
        {
            cout << "The thread joining is unsuccessful" << endl;
            return 1;
        }
    }

    pthread_mutex_destroy(&bridge_lock);
    return 0;
}