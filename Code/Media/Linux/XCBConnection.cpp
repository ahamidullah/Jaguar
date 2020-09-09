#include "XCBConnection.h"

xcb_connection_t *XCBConnection()
{
	static auto xcbConnection = (xcb_connection_t *){};
	if (!xcbConnection)
	{
		xcbConnection = xcb_connect(NULL, NULL);
    	if (auto rc = xcb_connection_has_error(xcbConnection); rc > 0)
    	{
    		Abort("XCBConnection", "Failed to open XCB connection: error code %d", rc);
    	}
	}
	return xcbConnection;
};
