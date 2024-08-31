#include <bits/stdc++.h>

using namespace std;

struct worker_thread
{
    int priority;
    int resources_available;
};

int n, m, requests;     

int find_thread(int worker_resources[], int curr_res)
{
    for(int i=0; i<m; i++)
    {
        if(curr_res <= worker_resources[i]) return i;
    }
    return -1;
}

void make_default(vector<vector<int>>& req_arr, vector<vector<int>>& wt_time)
{
    for(int i=0; i<n; i++)
    {
        for(int j=0; j<req_arr[i].size(); j++)
        {
            wt_time[i].push_back(0);
        }
    }
}

bool comp_func(const worker_thread &a, const worker_thread &b)
{
    return a.priority >= b.priority;
}

void queue_erase(vector<pair<int, int>>& queue_var, int a)
{
    for(int i=0; i<queue_var.size(); i++)
    {
        if(queue_var[i].second == a)
        {
            queue_var.erase(queue_var.begin()+i);
            return;
        }
    }
}

int out_of_values(vector<vector<worker_thread>>& prior_arr, int index, int value)
{
    int highest=0;
    for(int i=0; i<prior_arr[index].size(); i++)
    {
        if(highest < prior_arr[index][i].resources_available) highest = prior_arr[index][i].resources_available;
    }
    if(highest >= value) return 1;
    return -1;
}

int main()
{

    cout << "Enter the # of services" << endl;
    cin >> n;
    cout << "Enter the # of worker threads for each server" << endl;
    cin >> m;
    cout << "Enter the # of requests" << endl;
    cin >> requests;

    vector<vector<int>> req_arr(n);
    vector<vector<int>> wt_time(n);
    vector<vector<worker_thread>> priority_array;

    int temp[m], a1, a2, rejected = 0, waits=0;
    double avg_wt_time=0, avg_tat_time=0;

    cout << "Enter the priority level and resources assigned to a worker thread" << endl;
    for(int i=0; i<n; i++)
    {
        vector<worker_thread> alpha(m);
        for(int j=0; j<m; j++)
        {
            scanf("%d %d", &a1, &a2);
            alpha[j].priority = a1;
            alpha[j].resources_available = a2;
        }
        priority_array.push_back(alpha);
    }

    for(int i=0; i<n; i++)
    {
        sort(priority_array[i].begin(), priority_array[i].end(), comp_func);
    }

    cout << "Enter the transaction type and resources required for each request" << endl;
    int type,res;

    for(int i=0; i<requests; i++)
    {
        scanf("%d %d", &type, &res);
        req_arr[type].push_back(res);
    }
    make_default(req_arr, wt_time);

    printf("\nThe order of execution of requests is as follows: \n\n");
    for(int i=0; i<n; i++)
    {
        int time_stamp = 0, count = 1;

        vector<pair<int, int>> temp_queue; 

        for(int j=0; j<req_arr[i].size(); j++)
        {
            temp_queue.push_back(make_pair(j,req_arr[i][j]));
        }

        while(temp_queue.empty() != 1)
        {
            int place, worker_resources[m], iter_size = temp_queue.size(), abnormal;

            for(int k=0; k<m; k++)
            {
                worker_resources[k] = priority_array[i][k].resources_available;
            }

            for(int l=0; l<iter_size; l++)
            {
                int curr_res = temp_queue.front().second;               
                place = find_thread(worker_resources, curr_res);        
                abnormal = out_of_values(priority_array, i, curr_res);  
                if(place != -1 && abnormal != -1)
                {
                    printf("Request %d with resources %d under worker thread %d under service %d is executing\n", count, curr_res, place+1, i+1);
                    wt_time[i][l] = time_stamp;                     
                    queue_erase(temp_queue, curr_res);              
                    worker_resources[place] -= req_arr[i][l];       
                    count++;
                }
                else if(abnormal == -1)
                {
                    printf("The resources required (%d) to execute request %d is unavailable\n", curr_res, count);
                    wt_time[i][l] = -1;
                    queue_erase(temp_queue, curr_res);
                    count++;
                    rejected++;
                }
            }
            time_stamp++;    
        }
    }

    for(int i=0; i<n; i++)
    {
        for(int j=0; j<wt_time[i].size(); j++)
        {
            if(wt_time[i][j] > 0)
            {
                avg_wt_time += wt_time[i][j];
                waits++;
            }
        }
    }
    avg_wt_time /= requests;

    for(int i=0; i<n; i++)
    {
        for(int j=0; j<wt_time[i].size(); j++)
        {
            if(wt_time[i][j] != -1)
            {
                avg_tat_time += wt_time[i][j] + 1;
            }
        }
    }

    avg_tat_time /= requests;

  cout << "The average waiting time for all requests is " << avg_wt_time << endl;
cout << "The average turn around time for all requests is " << avg_tat_time << endl;
cout << "The # of requests rejected due to lack of resources is " << rejected << endl;
cout << "The # of requests that were forced to wait due to lack of available resources is " << waits << endl;

    return 0;
}