#ifndef OAUTH_H
#define OAUTH_H
#include <Arduino.h>

class OAuth
{
private:
    String accessToken;
    String refreshToken;
    String clientId;
    String clientSecret;

    void saveRefreshToken();
    void readRefreshToken();
public:
    OAuth(String _clientId, String _clientSecret);
    ~OAuth();

    void init();
    bool authWithAccessToken();
    bool authWithRefreshToken();

    String getAccessToken() {return accessToken;}
};


#endif /* OAUTH_H */