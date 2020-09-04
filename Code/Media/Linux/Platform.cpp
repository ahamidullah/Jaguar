#include "../Platform.h"

auto xcbConnection = (xcb_connection_t *){};

xcb_connection_t *XCBConnection()
{
	return xcbConnection;
};

void InitializePlatform()
{
    xcbConnection = xcb_connect(NULL, NULL);
    if (auto rc = xcb_connection_has_error(xcbConnection); rc > 0)
    {
    	Abort("Failed to open XCB connection: error code %d", rc);
    }
}
