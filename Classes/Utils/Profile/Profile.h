#ifndef PROFILE_H
#define PROFILE_H

#include <string>
class Profile {
private:
    static Profile* _instance;
    bool _isLogin;
    int _id;
    std::string _name;
    int _clansId;
    std::string _clansName;
public:
    static Profile* getInstance();
    static void destroyInstance();
    bool init();
    Profile();
    ~Profile();
    void setIsLogin(bool isLogin);
    void setId(int id);
    void setName(const std::string& name);
    void setClansId(int clansId);
    void setClansName(const std::string& clansName);
    bool getIsLogin() const;
    int getId() const;
    const std::string& getName() const;
    int getClansId() const;
    const std::string& getClansName() const;
    bool save();
};

#endif