#ifndef WEB_MANAGER_H
#define WEB_MANAGER_H

#include <WebServer.h>
#include "cal_manager/cal_manager.h"
#include "display_manager/display_manager.h"

class WebManager {
public:
    WebManager();
    ~WebManager();

    void init(CalManager *calManager, DisplayManager *display);
    void start();
    // Process incoming client connections (call regularly from loop())
    void handle();

private:
    WebServer server{80};
    CalManager *cal = nullptr;
    DisplayManager *dis = nullptr;
    void registerRoutes();
    bool pendingRefresh = false;
public:
    // If a refresh was requested via web UI, consume the flag and return true once
    bool consumeRefreshRequest();
};

#endif // WEB_MANAGER_H
