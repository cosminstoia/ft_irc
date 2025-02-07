#pragma once
#include <set>
#include <string>

class Channel 
{
    public:
        Channel(const std::string& name) : name_(name) {}

        // 
        void addMember(int socket) { members_.insert(socket); }
        void removeMember(int socket) { members_.erase(socket); }

        void addOperator(int socket) { operators_.insert(socket); }
        void removeOperator(int socket) { operators_.erase(socket); }

        bool isMember(int socket) { return members_.find(socket) != members_.end(); }
        bool isOperator(int socket) { return operators_.find(socket) != operators_.end(); }

        void setTopic(const std::string& newTopic) { topic_ = newTopic; }

        std::string const& getTopic() const { return topic_; }
        std::string const& getName() const { return name_; }
        std::set<int> const& getMembers() const { return members_; }
        std::set<int> const& getOperators() const { return operators_; }

    private:
        std::string name_;
        std::string topic_;
        std::set<int> members_;     // Store client sockets 
        std::set<int> operators_;   // Store client sockets of operators
};