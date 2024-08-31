#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <climits>
#include <string>
#include <set>
#include <stack>
#include <algorithm>


class Router {
private:
    int router_id;

public:
    std::vector<std::pair<Router*, int>> neighbors; // Pair of neighbor and link weight
    Router(int id) : router_id(id) {}
    std::map<int, Router*> routing_table;

    int getRouterID() const {
        return router_id;
    }

    void addNeighbor(Router* neighbor, int weight) {
        neighbors.push_back({neighbor, weight});
    }

    void updateRoutingTable(std::set<Router*> all_router_ptrs) {
        routing_table.clear();
        std::map<int, int> distance;
        std::map<Router*, Router*> next_hop;

        // Initialize distance to infinity and next_hop to NULL for all routers
        for (auto router : all_router_ptrs) {
            distance[router->getRouterID()] = 100000;
            next_hop[router] = NULL;
        }

        // Distance to self is 0
        distance[router_id] = 0;

        // Dijkstra's Algorithm
        std::priority_queue<std::pair<int, Router*>> pq;
        pq.push({0, this});

        while (!pq.empty()) {
            Router* node = pq.top().second;
            pq.pop();

            for (auto& neighbor : node->neighbors) {
                int v = neighbor.first->getRouterID();
                int weight = neighbor.second;

                if (distance[node->getRouterID()] + weight < distance[v]) {
                    distance[v] = distance[node->getRouterID()] + weight;
                    next_hop[neighbor.first] = node;
                    pq.push({-distance[v], neighbor.first});
                }
            }
        }

        // Set next hop to the router itself if no route exists
        for (Router* router_ptr : all_router_ptrs) {
            if (next_hop[router_ptr] == NULL) {
                next_hop[router_ptr] = router_ptr;
            }
        }

        std::set<Router*> neighbors_set;
        for (auto i : neighbors) {
            neighbors_set.insert(i.first);
        }

        // Build routing table for all routers (including self)
        for (const auto& entry : next_hop) {
            Router* dest_router = entry.first;
            Router* nh_router = entry.second;

            if (dest_router != nh_router && nh_router != this) {
                if(neighbors_set.find(nh_router)!=neighbors_set.end())
                {
                    routing_table[dest_router->getRouterID()] = nh_router;
                }
                else{
                    auto temp = nh_router;
                    while(neighbors_set.find(temp)==neighbors_set.end())
                    {
                        temp = next_hop[temp];
                    }
                    routing_table[dest_router->getRouterID()] = temp;
                }
                
            }
            else if(nh_router == this){
                routing_table[dest_router->getRouterID()] = dest_router;
                
            }
             else {
                routing_table[dest_router->getRouterID()] = NULL;
            }
        }
    }

    Router* getRouterByID(int id, const std::set<Router*>& all_router_ptrs) {
        for (Router* router_ptr : all_router_ptrs) {
            if (router_ptr->getRouterID() == id) {
                return router_ptr;
            }
        }
        return NULL;
    }

    void printRoutingTable() const {
        std::cout << "Routing Table for Router " << router_id << ":\n";
        for (const auto& entry : routing_table) {
            if(entry.first != router_id)
            {
                int dest = entry.first;
                Router* nh_router = entry.second;

                std::cout << "Destination: " << dest;
                if (nh_router != NULL) {
                    std::cout << " Next Hop: " << nh_router->getRouterID();
                } else {
                    std::cout << " Path does not exist";
                }
                std::cout << "\n";
                
            }
        }
        std::cout << "-------------------------\n";
    }
};

std::vector<Router*> findPath(int start, int finish,std::map<int,Router*> m) {
    if(start==finish)
    {
        return {m[start]};
    }
    auto s_router = m[start];
    auto f_router = m[finish];
    std::vector<Router*> v;
    int temp = start;
    while(temp != finish)
    {
        if(m[temp]->routing_table[finish] == NULL)
        {
            return {};
        }
        else{
            v.push_back(m[temp]);
            temp=m[temp]->routing_table[finish]->getRouterID();
        }
    }
    return v;

}

void createRouter(std::set<Router*>& all_router_ptrs) {
    int router_id;
    std::cout << "Enter the Router ID: ";
    std::cin >> router_id;

    // Check if the router ID already exists
    for (Router* router_ptr : all_router_ptrs) {
        if (router_ptr->getRouterID() == router_id) {
            std::cout << "Router with the same ID already exists.\n";
            return;
        }
    }

    // Create a new router with the specified ID
    Router* new_router = new Router(router_id);

    // Add incoming neighbors for the new router
    int num_incoming_neighbors;
    std::cout << "Enter the number of incoming neighbors: ";
    std::cin >> num_incoming_neighbors;

    for (int i = 0; i < num_incoming_neighbors; ++i) {
        int neighbor_id, weight;
        std::cout << "Enter incoming neighbor ID and weight for neighbor " << (i + 1) << ": ";
        std::cin >> neighbor_id >> weight;

        Router* neighbor_router = new_router->getRouterByID(neighbor_id, all_router_ptrs);

        if (neighbor_router) {
            neighbor_router->addNeighbor(new_router, weight);
        } else {
            std::cout << "Neighbor not found in the existing routers.\n";
        }
    }

    // Add outgoing neighbors for the new router
    int num_outgoing_neighbors;
    std::cout << "Enter the number of outgoing neighbors: ";
    std::cin >> num_outgoing_neighbors;

    for (int i = 0; i < num_outgoing_neighbors; ++i) {
        int neighbor_id, weight;
        std::cout << "Enter outgoing neighbor ID and weight for neighbor " << (i + 1) << ": ";
        std::cin >> neighbor_id >> weight;

        Router* neighbor_router = new_router->getRouterByID(neighbor_id, all_router_ptrs);

        if (neighbor_router) {
            new_router->addNeighbor(neighbor_router, weight);
        } else {
            std::cout << "Neighbor not found in the existing routers.\n";
        }
    }

    // Add the new router to the set of all routers
    all_router_ptrs.insert(new_router);
    std::cout << "Router " << router_id << " added successfully.\n";

    // Update routing tables for all routers
    for (Router* router_ptr : all_router_ptrs) {
        router_ptr->updateRoutingTable(all_router_ptrs);
    }
}

void deleteRouter(int del, std::set<Router*> &all_router_ptrs) {
    Router* routerToDelete = nullptr;

    // Find the router to delete
    for (Router* router_ptr : all_router_ptrs) {
        if (router_ptr->getRouterID() == del) {
            routerToDelete = router_ptr;
            break;
        }
    }

    if (routerToDelete == nullptr) {
        std::cout << "Router with ID " << del << " not found.\n";
        return;
    }

    // Remove the routerToDelete as a neighbor from other routers
    for (Router* router_ptr : all_router_ptrs) {
        if (router_ptr != routerToDelete) {
            auto it = router_ptr->neighbors.begin();
            while (it != router_ptr->neighbors.end()) {
                if (it->first == routerToDelete) {
                    it = router_ptr->neighbors.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

    // Erase routerToDelete from the set and deallocate memory
    all_router_ptrs.erase(routerToDelete);
    delete routerToDelete;

    // Update routing tables for all routers
    for (Router* router_ptr : all_router_ptrs) {
        router_ptr->updateRoutingTable(all_router_ptrs);
    }

    std::cout << "Router " << del << " has been deleted.\n";
}


int main() {
    std::set<Router*> all_router_ptrs;

    while (true) {
        std::cout<<"\n";
        std::cout << "Menu:\n";
        std::cout << "1. Create Router and Add Neighbors\n";
        std::cout << "2. Find Path\n";
        std::cout << "3. Print Routing Tables\n";
        std::cout << "4. Delete Router\n";
        std::cout << "5. Exit\n";
        std::cout << "Enter your choice: ";

        int choice;
        std::cin >> choice;

        switch (choice) {
            case 3: {

                std::cout << "Printing Routing Tables:\n";
                for (Router* router_ptr : all_router_ptrs) {
                    router_ptr->printRoutingTable();
                    std::cout << std::endl;
                }
                break;
            }
            case 2: {
                // Find and print the path between start and finish
                int start, finish;
                std::cout << "Enter start router ID: ";
                std::cin >> start;
                std::cout << "Enter finish router ID: ";
                std::cin >> finish;

                std::map<int, Router*> numtoptrs;
                for (Router* router_ptr : all_router_ptrs) {
                    numtoptrs.insert({router_ptr->getRouterID(), router_ptr});
                }

                std::vector<Router*> path = findPath(start, finish, numtoptrs);

                if (!path.empty()) {
                    std::cout << "Path from Router " << start << " to Router " << finish << ": ";
                    for (Router* router : path) {
                        std::cout << router->getRouterID() << " -> ";
                    }
                    std::cout << finish << std::endl;
                } else {
                    std::cout << "No path from Router " << start << " to Router " << finish << "." << std::endl;
                }
                break;
            }
            case 1: {
                // Create a new router and add neighbors
                createRouter(all_router_ptrs);
                break;
            }
            case 5: {
                // Exit the program
                // Clean up allocated memory for routers
                for (Router* router_ptr : all_router_ptrs) {
                    delete router_ptr;
                }
                return 0;
            }
            case 4: {
                //Delete the Router
                int del_router;
                std::cout<<"Enter Router id to Delete: ";
                std::cin>>del_router;
                deleteRouter(del_router,all_router_ptrs);
                break;
            }
            default: {
                std::cout << "Invalid choice. Please try again.\n";
                break;
            }
        }
    }

    return 0;
}