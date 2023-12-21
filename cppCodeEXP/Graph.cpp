#include "Graph.h"
#include <fstream>
#include <sstream>
#include <limits.h>
#include <queue>
#include <stack>
#include <functional>
#include <unordered_set>
#include <algorithm> 
using namespace std;

Graph::Graph(const char* const & edgelist_csv_fn) {
    ifstream my_file(edgelist_csv_fn);      
    string line;                     
    while(getline(my_file, line)) {      //parse file
        istringstream ss(line);      
        string u, v, weight; 
        getline(ss, u, ',');     
        getline(ss, v, ',');   
        getline(ss, weight, '\n');  
        double w = stod(weight);

        tuple<double,string,int> edge_u = make_tuple(w,v,0); // 0 if not duplicate edge, 1 if yes
        tuple<double,string,int> edge_v = make_tuple(w,u,1);

        if (graph.find(u) == graph.end()){     //create or update adjacency list for node u
            tuple<double,string,int> info_u = make_tuple(INT_MAX,"",-1);   //create info tuple. -1 represents not an edge
            vector<tuple<double,string,int>> edge_lst_u;
            edge_lst_u.push_back(info_u);
            edge_lst_u.push_back(edge_u);
            graph[u] = edge_lst_u; 
        } 
        else{
            graph[u].push_back(edge_u);
        }

        if (graph.find(v) == graph.end()){           //create or update adjacency list for node v
            tuple<double,string,int> info_v = make_tuple(INT_MAX,"",-1);   //create info tuple. -1 represents not an edge
            vector<tuple<double,string,int>> edge_lst_v;
            edge_lst_v.push_back(info_v);
            edge_lst_v.push_back(edge_v);
            graph[v] = edge_lst_v; 
        }
        else{
            graph[v].push_back(edge_v);
        }
    }
    my_file.close();    
}

unsigned int Graph::num_nodes() {
    unsigned int num_nodes = 0;
    for (auto it = graph.begin(); it != graph.end(); ++it){
        num_nodes++;
    }
    return num_nodes;
}

vector<string> Graph::nodes() {
    vector<string> keys;
    for (auto it = graph.begin(); it != graph.end(); ++it){
        keys.push_back(it->first);
    }
    return keys;
}

unsigned int Graph::num_edges() {
    unsigned int edges = 0;
    for (auto it = graph.begin(); it != graph.end(); ++it){
        auto i = it->second.begin();
        ++i;
        while (i != it->second.end()){
            if (get<2>(*i) == 0){  //if first time adding this edge
                edges++; 
            }
            ++i;
        }
    }
    return edges;
}

unsigned int Graph::num_neighbors(string const & node_label) {
    unsigned int neighbors = 0;
    if (graph.find(node_label) == graph.end()){
        return 0;
    }
    else{
        auto it = graph[node_label].begin(); 
        ++it;
        while (it != graph[node_label].end()){
            neighbors++;
            ++it;
        }
    }
    return neighbors;
}

double Graph::edge_weight(string const & u_label, string const & v_label) {
    if (graph.find(u_label) != graph.end()){
        auto it = graph[u_label].begin();
        ++it;
        while ( it != graph[u_label].end()){
            if (get<1>(*it) == v_label){
                return get<0>(*it);
            }
            ++it;
        }
    }
    return -1;
    
}

vector<string> Graph::neighbors(string const & node_label) {
    vector<string> neighbors;
    if (graph.find(node_label) != graph.end()){
        auto it = graph[node_label].begin();
        ++it; 
        while (it != graph[node_label].end()){
            neighbors.push_back(get<1>(*it));
            ++it; 
        }
    }
    return neighbors;
}

void Graph::clear_info(){
    vector<string> vertices = nodes();
    for (unsigned int i = 0; i < num_nodes(); i++){
        get<0>(graph[vertices[i]].front()) = INT_MAX;
        get<1>(graph[vertices[i]].front()) = "";
        get<2>(graph[vertices[i]].front()) = -1;
    }
}


vector<string> Graph::shortest_path_unweighted(string const & start_label, string const & end_label) {
    vector<string> path;
    if (start_label == end_label){
        path.push_back(start_label);
        return path;
    }
    if (graph.find(start_label) == graph.end()){
        return path;
    }
    clear_info();
    queue<string> toExplore;
    get<0>(graph[start_label].front()) = 0;  //set dist of start_label to 0;
    toExplore.push(start_label);
    while (!toExplore.empty()){
        string next = toExplore.front();
        toExplore.pop(); 
        double curr_dist = get<0>(graph[next].front());
        auto it = graph[next].begin(); //skip info tuple
        ++it; 
        while (it != graph[next].end()){  //add all its unvisited neighbors to the queue
            if (get<0>(graph[get<1>(*it)].front()) == INT_MAX){
                get<0>(graph[get<1>(*it)].front()) =  curr_dist + 1;   //update neighbor's dist
                get<1>(graph[get<1>(*it)].front()) = next;   //update neighbor's prev
                toExplore.push(get<1>(*it));    //push the name of neighbor to the queue
            }
            ++it;
        }
    }
    tuple<double,string,int> end_info = graph[end_label].front();
    if (get<1>(end_info) == ""){
        return path;
    }
    else{
        stack<string> stk;
        stk.push(end_label);
        tuple<double,string,int> curr_info = end_info;
        while (get<1>(curr_info) != start_label){   //while prev is not start_label
            stk.push(get<1>(curr_info));
            curr_info = graph[get<1>(curr_info)].front();
        }
        stk.push(start_label);
        while (!stk.empty()){
            path.push_back(stk.top());
            stk.pop();
        }
    }
    return path;
}
bool TupleCompare::operator()(tuple<string,double> a, tuple<string,double> b){
           return (get<1>(a) > get<1>(b));
}

vector<tuple<string,string,double>> Graph::shortest_path_weighted(string const & start_label, string const & end_label) {
     vector<tuple<string,string,double>> path;
    if (start_label == end_label){
        tuple<string,string,double> curr = make_tuple(start_label,start_label,-1);
        path.push_back(curr);
        return path;
    }
    if (graph.find(start_label) == graph.end()){
        return path;
    }
    clear_info();
    priority_queue<tuple<string,double>,vector<tuple<string,double>>, TupleCompare> toExplore;
    get<0>(graph[start_label].front()) = 0;  //set dist of start_label to 0;
    tuple<string,double> start = make_tuple(start_label, 0);
    toExplore.push(start);
    while (!toExplore.empty()){
        tuple<string,double> curr = toExplore.top();
        toExplore.pop(); 
        double curr_dist = get<1>(curr);
        auto it = graph[get<0>(curr)].begin(); //skip info tuple
        if (get<2>(*it) == 1){  //if current node is done visiting, continue to the next node
            continue;
        }
        get<2>(*it) = 1;  //mark current node done
        ++it; 
        while (it != graph[get<0>(curr)].end()){  //add all its unvisited neighbors to the queue
            if (curr_dist + get<0>(*it) < get<0>(graph[get<1>(*it)].front())){  //get<1>(*it): name of the neighbor; get<0>(*it): weight to that neighbor
                get<0>(graph[get<1>(*it)].front()) =  curr_dist + get<0>(*it);   //update neighbor's dist = curr_dist + weight
                get<1>(graph[get<1>(*it)].front()) = get<0>(curr);   //update neighbor's prev
                tuple<string,double> next = make_tuple(get<1>(*it),  get<0>(graph[get<1>(*it)].front()));
                toExplore.push(next);    //push the name of neighbor to the queue
            }
            ++it;
        }
    }
    tuple<double,string,int> end_info = graph[end_label].front();
    if (get<1>(end_info) == ""){
        return path;
    }
    else{
        stack<tuple<string,string,double>> stk;
        tuple<double,string,int> curr_info = end_info;
        tuple<double,string,int> prev_info = graph[get<1>(curr_info)].front();
        tuple<string,string,double> curr = make_tuple(get<1>(curr_info),end_label,get<0>(curr_info) - get<0>(prev_info)); //0 is dist and 1 is prev
        stk.push(curr);  
        while (get<1>(curr_info) != start_label){   //while prev is not start_label
            prev_info = graph[get<1>(curr_info)].front();
            get<0>(curr) = get<1>(prev_info);
            get<1>(curr) = get<1>(curr_info);
            get<2>(curr) = get<0>(prev_info) - get<0>(graph[get<1>(prev_info)].front());
            stk.push(curr);
            curr_info = prev_info;  //set curr_info to the info tuple of its prev
        }
        while (!stk.empty()){
            path.push_back(stk.top());
            stk.pop();
        }
    }
    return path;
}

vector<vector<string>> Graph::connected_components(double const & threshold) {
    clear_info();
    vector<vector<string>> output;
    vector<string> vetices = nodes();
    unordered_set<string> unvisited;
    for (auto it = vetices.begin(); it != vetices.end(); ++it){
        unvisited.insert(*it);
    } 
    while (!unvisited.empty()){
        auto first = unvisited.begin();    
        string curr = *first;
        unvisited.erase(*first);     //visit the first node
        vector<string> subset;       //create a subset including that node 
        subset.push_back(curr);
        queue<string> toExplore;
        get<0>(graph[curr].front()) = 0;  //set dist of start_label to 0;
        toExplore.push(curr);
        while (!toExplore.empty()){
            string next = toExplore.front();
            toExplore.pop(); 
            double curr_dist = get<0>(graph[next].front());
            auto it = graph[next].begin(); //skip info tuple
            ++it; 
            while (it != graph[next].end()){  
                if (get<0>(graph[get<1>(*it)].front()) == INT_MAX && get<0>(*it) <= threshold){  //visit if not visited and edge weight <= thresh
                    get<0>(graph[get<1>(*it)].front()) =  curr_dist + 1;   //update neighbor's dist
                    subset.push_back(get<1>(*it));
                    unvisited.erase(get<1>(*it));
                    toExplore.push(get<1>(*it));    //push the name of neighbor to the queue
                }
            ++it;
            }
        }
        output.push_back(subset);
    }
    return output;

}

double Graph::smallest_connecting_threshold(string const & start_label, string const & end_label) {
    if (start_label == end_label){
        return 0;
    }
    if (graph.find(start_label) == graph.end()){
        return -1;
    }
    clear_info();
    unordered_map<string, vector<tuple<double,int,string>>> sets;  //<node, tuple(weight,size,parent)>
    vector<tuple<double, string, string>> weights;   //<weight, start vertex, end vertex>
    for (auto it = graph.begin(); it != graph.end(); ++it){  
        string start_v = it->first;
        tuple<double,int,string> node = make_tuple(-1,1,""); //initialize a node
        sets[start_v].push_back(node);
        auto i = it->second.begin(); 
        ++i;
        while (i != it->second.end()){
            if (get<2>(*i) == 0) {
                tuple<double, string, string> w = make_tuple(get<0>(*i), start_v, get<1>(*i));
                weights.push_back(w);
            }
            ++i;
        }
    }
    sort(weights.begin(),weights.end());  //sort the weights in ascending order 

    auto find = [&] (string node) -> string{  //find with path compression
        string curr = node;
        vector<string> children;
        while (get<2>(sets[curr].front()) != ""){
            children.push_back(curr);
            curr = get<2>(sets[curr].front());
        }
        for (auto it = children.begin(); it != children.end(); ++it){
            get<2>(sets[*it].front()) = curr;
        }
        return curr;
    };
    while (!weights.empty()){
        tuple<double, string, string> curr = weights.front();
        double weight = get<0>(curr);
        string start_v = get<1>(curr);
        string end_v = get<2>(curr);
        weights.erase(weights.begin());
        string s_v = find(start_v);  //get the sentinal nodes
        string e_v = find(end_v);
        if (s_v == e_v){  //if they are already in the same set (the function would return in previous iteration since weight sorted ascendingly)
            continue;
        }
        if (get<1>(sets[s_v].front()) >= get<1>(sets[e_v].front())){   //union on the two sentinals set uptree with greater or equal size as sentinal
            get<2>(sets[e_v].front()) = s_v;
            string s_v_a = get<2>(sets[e_v].front());
            get<1>(sets[s_v].front()) += get<1>(sets[e_v].front());
            get<0>(sets[s_v].front()) = weight;
        }
        else{
            get<2>(sets[s_v].front()) = e_v;
            get<1>(sets[e_v].front()) += get<1>(sets[s_v].front());
            get<0>(sets[e_v].front()) = weight;
        }
        if (find(start_label) == find(end_label)){
            return get<0>(sets[find(start_label)].front());
        }
    }
    return -1;
    
}
