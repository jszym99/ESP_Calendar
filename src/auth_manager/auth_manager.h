#ifndef AUTH_MANAGER_H
#define AUTH_MANAGER_H
#include <Arduino.h>
#include <ArduinoJson.h>

class AuthManager
{
private:
    String accessToken;
    String refreshToken;
    String clientId;
    String clientSecret;
    uint16_t tokenLifetime;

    void saveRefreshToken();
    void readRefreshToken();
    bool authWithDeviceCode();
    bool authWithRefreshToken();

public:
    AuthManager(String _clientId, String _clientSecret);
    ~AuthManager();

    bool authenticate();

    String getAccessToken() {return accessToken;}
    uint16_t getTokenLifetime() {return tokenLifetime;}
};


#endif /* AUTH_MANAGER_H */