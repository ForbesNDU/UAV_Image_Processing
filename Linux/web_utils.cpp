#include "web_utils.h"

// Send request to GoPro
bool ping_url(std::string url) {

	CURL *curl;
	CURLcode res;

	curl = curl_easy_init();
	
	if(curl) {
		curl_easy_setopt( curl, CURLOPT_URL, url.c_str() );
		res = curl_easy_perform( curl );

		if(res != CURLE_OK) {
			std::cout << "An error occurred processing a curl request: " << curl_easy_strerror(res) << std::endl;
			return false;
		}

		curl_easy_cleanup(curl);
	} 
	else {
		std::cout << "Failed to initialize CURL" << std::endl;
		return false;
	}

}
