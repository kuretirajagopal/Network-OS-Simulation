#include <bits/stdc++.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <chrono>
using namespace std;
using namespace std::chrono; // For introducing system time to the program

#define events 100
#define capacity 500
#define worker_threads 20
#define MAX 5

// Initializing the events array
int events_arr[events] = {0}, concurrency=0;
vector<vector<int>> shared_table(MAX, vector<int>(3, -1));   

// Initializing the lock variables and condition variables
pthread_mutex_t table_lock = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t concurrency_lock = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t table_cond = PTHREAD_COND_INITIALIZER;

// Initializing the thread id array for the worker threads
pthread_t tid[worker_threads];
vector<int> bookings;

int exec_time = rand() % 10 + 1;
auto finish = system_clock::now() + minutes{1};


// Function to make shared table entry to default
void table_default(int row_no)
{
    pthread_mutex_lock(&table_lock);
    shared_table[row_no][0] = -1;
    shared_table[row_no][1] = -1;
    shared_table[row_no][2] = -1;
    pthread_mutex_unlock(&table_lock);
}

// Finding whether the event is present in the shared table or not
int event_finder(int event_no)
{
    int res=-1;
    for(int i=0; i<MAX; i++)
    {
        pthread_mutex_lock(&table_lock);
        if(shared_table[i][0] == event_no) res = i;
        pthread_mutex_unlock(&table_lock);
    }
    return res;
}

// Function to inquire about the tickets
void enquiry(int event_no)
{
    int entry = event_finder(event_no);

    if(entry != -1)
    {
        if(shared_table[entry][1] != 1)
        {
            shared_table[entry][2]++;
            cout <<pthread_self()<<": the avaliable # of seats in event " << event_no << " are " << 500 - events_arr[event_no] << endl;
            sleep(rand() % 10 + 1);
        }

        else
        {
            pthread_mutex_lock(&table_lock);
            while(shared_table[entry][1] == 1)
            {
                pthread_cond_wait(&table_cond, &table_lock);
            }
            pthread_mutex_unlock(&table_lock);
        }
    }
    else
    {
        int index = event_finder(-1);
        shared_table[index][0] = event_no;
        shared_table[index][1] = 0;
        shared_table[index][2]++;
        cout <<pthread_self()<<": the avaliable # of seats in event " << event_no << " are " << 500 - events_arr[event_no] << endl;
        sleep(rand() % 5 + 1);
    }

    // Handling the locks for concurrency and table modification
    pthread_mutex_lock(&concurrency_lock);
    concurrency--;
    entry = event_finder(event_no);
    if(entry != -1)table_default(entry);
    pthread_cond_signal(&cond);
    pthread_cond_signal(&table_cond);
    pthread_mutex_unlock(&concurrency_lock);
}

// Function to book tickets
void book_tickets(int event_no, int total_seats)
{
    int entry = event_finder(event_no);
    if(entry != -1)
    {
        pthread_mutex_lock(&table_lock);
        while(shared_table[entry][1] == 1 || shared_table[entry][1]==0)
        {
            pthread_cond_wait(&table_cond, &table_lock);
        }
        pthread_mutex_unlock(&table_lock);  
    }
    int index = event_finder(-1);
    shared_table[index][0] = event_no;
    shared_table[index][1] = 1;
    shared_table[index][2]++;

    if(events_arr[event_no] + total_seats <= capacity)
    {
        events_arr[event_no] += total_seats;
        cout <<pthread_self()<<": Booking done. The # of seats available in event " << event_no << " are " << 500 - events_arr[event_no] << endl;
        sleep(rand() % 5 + 1);
    }
    else
    {
        cout <<pthread_self()<<": Max capacity has been reached" << endl;
        sleep(rand() % 5 + 1);
    }
    pthread_mutex_lock(&concurrency_lock);
    concurrency--;
    entry = event_finder(event_no);
    if(entry != -1)table_default(entry);
    pthread_cond_signal(&table_cond);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&concurrency_lock);
}

// Function to cancel a booked ticket
void cancel_ticket(int event_no)
{
    int entry = event_finder(event_no);
    if(entry != -1)
    {
        pthread_mutex_lock(&table_lock);
        while(shared_table[entry][1] == 1 || shared_table[entry][1]==0)
        {
            pthread_cond_wait(&table_cond, &table_lock);
        }
        pthread_mutex_unlock(&table_lock);
    }

    int index = event_finder(-1);
    shared_table[index][0] = event_no;
    shared_table[index][1] = 1;
    shared_table[index][2]++;
    cout <<pthread_self()<<": Before cancelling the ticket, the total tickets available in event " << event_no << " are " << 500 - events_arr[event_no] << endl;
    events_arr[event_no]--;
    cout <<pthread_self()<<": After cancelling the ticket, the total tickets available in event " << event_no << " are " << 500 - events_arr[event_no] << endl;
    sleep(rand() % 5 + 1);
    pthread_mutex_lock(&concurrency_lock);
    concurrency--;
    entry = event_finder(event_no);
    if(entry != -1)table_default(entry);
    pthread_cond_signal(&table_cond);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&concurrency_lock);
}


// Function for automatically generating the queries
void* query_function(void* argv)
{
    
    while(system_clock::now() < finish)
    {
        // Handling the concurrency 
        if(concurrency < MAX)
        {
            int query_selector = rand() % 3 + 1;
            int event_no = rand() % events;
            int no_of_seats = rand() % capacity + 1;
            
            if(query_selector == 1)
            {
                concurrency++;
                enquiry(event_no);
            }
            else if(query_selector == 2)
            {
                concurrency++;
                bookings.push_back(event_no);
                book_tickets(event_no, no_of_seats);
            }
            else 
            {
                if(bookings.size() == 0)
                {
                    cout <<pthread_self()<<": No bookings are made yet by this thread" << endl;
                }
                else
                {
                    int tkt_cancel = rand() % bookings.size();
                    concurrency++;
                    cancel_ticket(bookings[tkt_cancel]);
                }
                
            }
        }
        else 
        {
            pthread_mutex_lock(&concurrency_lock);
            while(concurrency >= MAX)
            {
                pthread_cond_wait(&cond, &concurrency_lock);
            }
            pthread_mutex_unlock(&concurrency_lock); 
        }
        sleep(rand() % 10 + 1);
        
    }

    // Waiting for the threads to finish their queries 
    pthread_join(pthread_self(), NULL);
    return 0;
}

int main(void)
{

    int error_no;
    srand(100);

    // Creating the  threads
    for(int i=0; i<worker_threads; i++)
    {
        error_no = pthread_create(&(tid[i]), NULL, &query_function, NULL);
        if(error_no != 0)
        {
            cout <<pthread_self()<<": Thread can't be created with error: " << strerror(error_no);
        }
    }
    pthread_cond_destroy(&cond);
    pthread_cond_destroy(&table_cond);
    pthread_mutex_destroy(&table_lock);
    pthread_mutex_destroy(&concurrency_lock);

    // Final reservation status
    cout <<pthread_self()<<": The final reservation status of all events is as follows: " << endl;
    for(int i=0; i<events; i++)
    {
       cout <<pthread_self()<<": The # of seats booked in event " << i << " are " << events_arr[i] << endl;
    }

    return 0;
}