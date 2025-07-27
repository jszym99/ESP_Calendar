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
    bool checkResponse(JsonDocument response);
public:
    AuthManager(String _clientId, String _clientSecret);
    ~AuthManager();

    void init();
    bool authWithAccessToken();
    bool authWithRefreshToken();

    String getAccessToken() {return accessToken;}
};


#endif /* AUTH_MANAGER_H */