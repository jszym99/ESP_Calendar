#ifndef AUTH_MANAGER_H
#define AUTH_MANAGER_H
#include <Arduino.h>
#include <ArduinoJson.h>

constexpr uint16_t MAX_TOKEN_LEN = 256;

class AuthManager
{
private:
    char accessToken[MAX_TOKEN_LEN];
    char refreshToken[MAX_TOKEN_LEN];
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

    const char * getAccessToken() const {return accessToken;}
    uint16_t getTokenLifetime() {return tokenLifetime;}
};


#endif /* AUTH_MANAGER_H */