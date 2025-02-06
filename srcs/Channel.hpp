#include <unordered_map>
#include <set>
#include <string>
#include <vector>

// Example Channel structure
struct Channel 
{
    std::string name;
    std::string topic;
    std::set<int> members; // Store client sockets or IDs
    // Add more properties as needed (e.g., channel modes, keys)
};