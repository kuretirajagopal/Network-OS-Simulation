#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unistd.h>
#include <chrono>

using namespace std;

const int N = 10; 

mutex m; 
condition_variable cv_front, cv_back; 

bool front = false; 
bool back = false; 
int diners_inside = 0; 
int diners_served = 0; 


void diner_thread(int id)
{
    while(true)
    {
        unique_lock<mutex> lock_var(m);

        while (!front) 
        {
            cv_front.wait(lock_var);
        }

        diners_inside++;
        cout << "Diner " << id << " entered the restaurant.\n";

        if (diners_inside == N) 
        {
            front = false;
            cv_front.notify_all();
            sleep(3);
            back = true;
            cv_back.notify_all();
            cout << "All diners inside. Service starting.\n";

        }

        else 
        {
            cv_back.wait(lock_var);

        }

        diners_inside--;
        diners_served++;
        cout << "Diner " << id << " exited the restaurant.\n";

        if (diners_inside == 0) {
            back = false;
            cv_back.notify_all();
            cout << "All diners served. Next batch can enter.\n";
            diners_served = 0;
        }
    }

}

int main()
{

    thread diners[N*2];
    for (int i = 0; i < N*2; i++) 
    {
        diners[i] = thread(diner_thread, i);
    }

    while (true) 
    {
        unique_lock<mutex> lock_var(m);

        front = true;
        cv_front.notify_all();
        cout << "Restaurant is open. Waiting for diners to enter...\n";

        if (diners_inside < N) {
            cv_front.wait(lock_var);
        }

        if (!back) {
            cv_back.wait(lock_var);
        }

        if (diners_inside > 0) {
            cv_back.wait(lock_var);
        }
    }
    for (int i = 0; i < N*2; i++) 
    {
        diners[i].join();
    }

    return 0;
}
