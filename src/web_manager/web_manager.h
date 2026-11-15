#ifndef WEB_MANAGER_H
#define WEB_MANAGER_H

#include <FS.h>
#include <WebServer.h>
#include <ElegantOTA.h>
#include "cal_manager/cal_manager.h"

class WebManager {
public:
    WebManager();
    ~WebManager();

    void init(CalManager *calManager);
    void start();
    // Process incoming client connections (call regularly from loop())
    void handle();
    // If a refresh was requested via web UI, consume the flag and return true once
    bool consumeRefreshRequest();

private:
    WebServer server{80};
    CalManager *cal = nullptr;
    bool pendingRefresh = false;
    void registerRoutes();
};

#endif // WEB_MANAGER_H
