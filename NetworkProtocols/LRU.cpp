#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include<netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <regex>

using namespace std;



std::string SendHttpGetRequest(const std::string& host, const std::string& path) {
    int port = 80; // HTTP port

    // Resolve the hostname to an IP address using gethostbyname
    struct hostent* hostInfo;
    hostInfo = gethostbyname(host.c_str());
    if (hostInfo == nullptr) {
        std::cerr << "Error: Unable to resolve hostname" << std::endl;
        return "1";
    }

    // Create a socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Error: Failed to create socket" << std::endl;
        return "1";
    }

    // Set up server address and port
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    memcpy(&serverAddr.sin_addr, hostInfo->h_addr, hostInfo->h_length);

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error: Failed to connect to server" << std::endl;
        close(clientSocket);
        return "1";
    }

    std::string httpRequest = "GET " + std::string(path) + " HTTP/1.1\r\n"
                              "Host: " + std::string(host) + "\r\n"
                              "Connection: close\r\n\r\n";

      // Send the HTTP GET request
    if (send(clientSocket, httpRequest.c_str(), httpRequest.size(), 0) == -1) {
        std::cerr << "Error: Failed to send HTTP request" << std::endl;
        close(clientSocket);
        return "1";
    }

    // Receive and print the HTTP response
    char buffer[4096];
    string s;
    ssize_t bytesRead;
    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytesRead] = '\0';
        s+=buffer;
        //std::cout << buffer;
    }
     
    if (bytesRead == -1) {
        std::cerr << "Error: Failed to receive data" << std::endl;
    }
    close(clientSocket);
    
    //cout<<s;
    return s;
}

// LRU Cache using Linked List
 class Node{
        public: 
           string key;//url
            string val;//response
            Node* prev;
            Node* next;

            Node(string key,string val){
               this->key=key;
                this->val = val;
            }
    };

class LRUCache {
public:
     Node* head = new Node("-1","-1");
    Node* tail = new Node( "-1","-1");

    int cap;
    map<string, Node*> m;// Ammortized cost = O(1)

    LRUCache(int capacity) {
      cap = capacity;
        head -> next = tail;
        tail -> prev = head;
    }
// Adding a new node into cache
    void addNode(Node * newnode){
        auto next=head->next;
        head->next=newnode;
        newnode->prev=head;
        newnode->next=next;
        next->prev=newnode;
    }
// Deleting a node from Cache
    void deleteNode(Node* delnode){
        Node* prevv = delnode -> prev;
        Node* nextt = delnode -> next;
        prevv -> next = nextt;
        nextt -> prev = prevv;
    }
// Access a node of cache
    string get(string key) {
       if(m.find(key)!=m.end()){
           Node* resNode = m[key];
            string ans = resNode -> val;
            deleteNode(resNode);
            addNode(resNode);
            m[key] = head -> next;
            return ans;
        }
        return "-1";
       }
// Put a value into cache
    void put(string key, string value) {
        
         if(m.size() == cap){
            m.erase(tail -> prev -> key);
            deleteNode(tail -> prev);
        }
        addNode(new Node(key, value));
        m[key] = head -> next;
    }
            
    
};

void print(Node *head){
    cout<<"Cache Contents (Most to Least Recently Used\n";
    int i=1;
    while(head!=NULL){
        if(head->val=="-1");
        else{
        cout<<i<<" "<<head->key<<endl;
        i++;
        }
        head=head->next;
    }
}



// Function to extract host and path from a URL
bool ExtractHostAndPath(const std::string& url, std::string& host, std::string& path) {
    // Define a regular expression pattern for a basic URL
    std::regex urlPattern("^(http?://)?([^/]+)(/.*)?$");
    std::smatch match;
    

    // Attempt to match the URL against the pattern
    if (std::regex_match(url, match, urlPattern)) {
        if (match[2].matched) {
            host = match[2].str();
        }

        if (match[3].matched) {
            path = match[3].str();
        } else {
            path = "/";
        }

        return true;
    } else {
        return false; // URL does not match the expected format
    }
}

int main(){
   
    cout<<"Enter the capacity\n";
    int cap;
    cin>>cap;
    
    LRUCache cache(cap);
    while(1){
        cout<<"Enter URL (or 'exit' to quit): ";
        string s;
        cin>>s;
        if(s=="exit"){
            cout<<"Exiting....\n";
            break;
        }
        string host;
        string path;
        int b=ExtractHostAndPath(s,host,path);
        if(!b){
            cerr<<"Error in extracting the path and host\n";
            continue;
        }
        
        string gt=cache.get(s);
        string ans;
        // if not present in cache
        if(gt=="-1"){
            ans=SendHttpGetRequest(host,path);
            cache.put(s,ans);
        }
        // if present in cache
        else{
           ans=gt;

        }
        cout<<"Page Content:  \n";
        
            cout<<ans<<endl;

       print(cache.head);
cout<<endl;
    
}


}