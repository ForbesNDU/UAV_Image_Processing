#include "web_utils.h"

// Send request to GoPro
bool ping_request(HINTERNET hConnect, LPCWSTR request) {
		HINTERNET hRequest = WinHttpOpenRequest( hConnect, L"GET", 
											 request, 
                                             NULL, WINHTTP_NO_REFERER, 
                                             WINHTTP_DEFAULT_ACCEPT_TYPES,
                                             0);

	if(hRequest == NULL) {
		printf("Error %u in WinHttpOpenRequest\n", GetLastError());
		std::cin.get();
		return true;
	}

    bool bResults = WinHttpSendRequest( hRequest, 
                                        WINHTTP_NO_ADDITIONAL_HEADERS,
                                        0, WINHTTP_NO_REQUEST_DATA, 0, 
                                        0, 0);

	if (!bResults) {
		printf( "Error %u in WinHttpSendRequest\n", GetLastError());
		std::cin.get();
		return true;
	}
	WinHttpCloseHandle(hRequest);

	return false;
}
