#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>

int globalCollisionCounter = 0;
float transmissionProab;


// Node class representing network nodes
class Node {
public:
    int id;
    int backoffInterval;
    int collisionCount;
    int successfulTransmissions;
    bool istransmitting;
    std::vector<int> collisionsBeforeSuccess;
    std::vector<int> timeBeforeSuccess;

    Node(int nodeId) : id(nodeId), backoffInterval(0), collisionCount(0), successfulTransmissions(0) {}

    // Implement a method to perform backoff
    void performBackoff(int maxBackoff) {
        // Generate a random backoff interval between 0 and 2^collisionCount-1
        backoffInterval = (std::rand() % std::min((1 << collisionCount), 1 << maxBackoff))+1;
    }

    void performBusyBackoff(int backoffChannelBusy) {
        backoffInterval = (std::rand() % backoffChannelBusy)+1;
    }

    // Implement a method to simulate transmission attempt
    bool attemptTransmission() {
        if (backoffInterval == 0) {
            // Node attempts transmission
            return true;
        }
        // Node is still in backoff
        return false;
    }

    bool transmittingProbability()
    {
        return (rand()%100 < transmissionProab*100);
    }
};

// Function to simulate the CSMA protocol
void simulateCSMA(int numNodes, int maxBackoff, int simulationDuration, int maxtransmissionTime, int backoffChannelBusy) {
    std::vector<Node> nodes;
    

    // Initialize nodes
    for (int i = 0; i < numNodes; ++i) {
        nodes.push_back(Node(i));
    }
    std::vector<int> countcollisions(nodes.size());
    std::vector<int> counttime(nodes.size());
    int channelbusynode = -1;
    int transmissionTime = (std::rand()%maxtransmissionTime)+2;
    bool channelBusy = false;
    int transmissionTimer = 0;
   
    for (int time = 0; time < simulationDuration; ++time) {
        for(Node &node : nodes)
        {
            if(!node.istransmitting)
            {
                node.istransmitting=node.transmittingProbability();
            }
            
        }
        
        if (channelBusy) {
            transmissionTimer++;
                for(Node &node : nodes) {
                    if(channelbusynode != node.id && node.attemptTransmission())
                    {
                        if(node.istransmitting)
                        node.performBusyBackoff(backoffChannelBusy);
                    }
                    else{
                        // Node is still in backoff
                        if(node.backoffInterval>0)
                        node.backoffInterval--;
                    }
                }

                if (transmissionTimer == transmissionTime) {
                transmissionTimer = 0;
                channelBusy = false;
                transmissionTime = (std::rand()%maxtransmissionTime)+2;
                }
                continue;

        }
        for (Node &node : nodes) {
            if (node.attemptTransmission() && node.istransmitting) {
                // Node attempts transmission
                // Check for collisions with other nodes
                int collisions = 0;
                for (Node &other : nodes) {
                    if (node.id != other.id && other.attemptTransmission() && other.istransmitting) {
                        // Collision detected
                        countcollisions[other.id]++;
                        collisions++;
                        other.collisionCount++;
                        other.performBackoff(maxBackoff);
                    }
                }

                if (collisions == 0) {
                    // Successful transmission
                    node.istransmitting=node.transmittingProbability();
                    node.collisionsBeforeSuccess.push_back(countcollisions[node.id]);
                    countcollisions[node.id]=0;
                    node.timeBeforeSuccess.push_back(time);
                    channelBusy = true;
                    channelbusynode = node.id;
                    transmissionTimer++;
                    node.successfulTransmissions++;
                } else {
                    // Collision occurred
                    countcollisions[node.id]++;
                    globalCollisionCounter++;
                    node.collisionCount++;
                    node.performBackoff(maxBackoff);
                }
            } else {
                //Node is still in backoff
                if(node.backoffInterval>0)
                node.backoffInterval--;
            }
        }
    }


    // Print statistics
    for (const Node &node : nodes) {
        std::cout << "Node " << node.id << " - Successful Transmissions: " << node.successfulTransmissions
             << " Collisions: " << node.collisionCount << std::endl;
    }
    std::cout<<"\nMore Stastics\n";
        for (const Node &node : nodes) {
        std::cout << "Node " << node.id  << ":\n";
        std::cout<<"collisions Before Successfull transmission:\n";
        for(int i=0;i<node.collisionsBeforeSuccess.size();i++)
        {
            std::cout<<node.collisionsBeforeSuccess[i]<<" ";
        }
        std::cout<<"\ntime Before Successfull transmission:\n";
        for(int i=0;i<node.timeBeforeSuccess.size();i++)
        {
            std::cout<<node.timeBeforeSuccess[i]<<" ";
        }
        std::cout<<"\n";
    }
}

int main() {
    int numNodes, maxBackoff, simulationDuration, maxtransmissionTime, backoffChannelBusy;

    // User input for configuration parameters
    std::cout << "Enter the number of nodes: ";
    std::cin >> numNodes;
    std::cout << "Enter the maximum backoff value: ";
    std::cin >> maxBackoff;
    std::cout << "Enter the backoff for channel being idle  value: ";
    std::cin >> backoffChannelBusy;
    std::cout << "Enter the simulation duration: ";
    std::cin >> simulationDuration;
    std::cout << "Enter the max transmission time: ";
    std::cin >> maxtransmissionTime;
    std::cout << "Enter Transmission Probability (0 to 1): ";
    std::cin >> transmissionProab;
    simulateCSMA(numNodes, maxBackoff, simulationDuration, maxtransmissionTime, backoffChannelBusy);
    std::cout << "Global Collision Counter: " << globalCollisionCounter << '\n';

    return 0;
}
