#pragma once
#include <set>
#include <string>

class Channel 
{
    public:
        Channel() : name_(""), topic_() {}
        Channel(const std::string& name) : name_(name), topic_() {}

        void addMember(int socket) { members_.insert(socket); }
        void removeMember(int socket) { members_.erase(socket); }
        void addOperator(int socket) { operators_.insert(socket); }
        void removeOperator(int socket) { operators_.erase(socket); }
        bool isMember(int socket) const { return members_.find(socket) != members_.end(); }
        bool isOperator(int socket) { return operators_.find(socket) != operators_.end(); }
        void setTopic(const std::string& newTopic) { topic_ = newTopic; }
        std::string const& getTopic() const { return topic_; }
        std::string const& getName() const { return name_; }
        std::set<int> const& getMembers() const { return members_; }
        std::set<int> const& getOperators() const { return operators_; }
        bool isTopicRestricted() const { return topicRestricted_; }
        void topicRestricted(bool restricted) { topicRestricted_ = restricted; }
        std::string const& getPassword() const { return password_; }
        void setPassword(const std::string& password) { password_ = password; }
        void setUserLimit(int limit) { userLimit_ = limit; }
        int getUserLimit() const { return userLimit_; }
        bool isFull() const { return members_.size() >= static_cast<size_t>(userLimit_); }
        void removeClient(int socket) { members_.erase(socket); }
        bool isClientInChannel(int socket) const { return members_.find(socket) != members_.end(); }
        bool isInviteOnly() const { return isInviteOnly_; }
        void setInviteOnly(bool inviteOnly) { isInviteOnly_ = inviteOnly; }
        void addInvite(int socket) { inviteList_.insert(socket); }
        void removeInvite(int socket) { inviteList_.erase(socket); }
        bool isInvited(int socket) const { return inviteList_.find(socket) != inviteList_.end(); }

    private:
        std::string name_;
        std::string topic_;
        std::string password_;
        std::set<int> members_;
        std::set<int> operators_;
        bool isInviteOnly_ = false;
        bool topicRestricted_ = false;
        int userLimit_ = USER_LIMIT_CHANNEL;
        std::set<int> inviteList_; // List of clients invited to the channel
};