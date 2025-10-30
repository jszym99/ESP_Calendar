#ifndef AUTH_MANAGER_H
#define AUTH_MANAGER_H
#include <Arduino.h>
#include <ArduinoJson.h>

class AuthManager
{
private:
    std::string accessToken;
    std::string refreshToken;
    std::string clientId;
    std::string clientSecret;
    uint16_t tokenLifetime;

    void saveRefreshToken();
    void readRefreshToken();
    bool authWithDeviceCode();
    bool authWithRefreshToken();

public:
    AuthManager(std::string _clientId, std::string _clientSecret);
    ~AuthManager();

    bool authenticate();

    std::string getAccessToken() {return accessToken;}
    uint16_t getTokenLifetime() {return tokenLifetime;}
};


#endif /* AUTH_MANAGER_H */