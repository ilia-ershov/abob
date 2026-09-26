#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <regex.h>

typedef struct {
	char *data;
	size_t length;
	size_t capacity;
} response_buffer;

static response_buffer downloaded = {0};

static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	response_buffer *buffer = userdata;
	size_t received = size * nmemb;

	if (buffer->length + received + 1 > buffer->capacity) {
		size_t capacity = buffer->capacity == 0 ? 8192 : buffer->capacity;
		while (capacity < buffer->length + received + 1)
			capacity *= 2;
		char *data = realloc(buffer->data, capacity);
		if (data == NULL)
			return 0;
		buffer->data = data;
		buffer->capacity = capacity;
	}

	memcpy(buffer->data + buffer->length, ptr, received);
	buffer->length += received;
	buffer->data[buffer->length] = '\0';
	return received;
}

int validate_url(char *url)
{
	static const char *pattern = "^https?://[^[:space:]/]+([/?].*)?$";
	regex_t regex;

	if (url == NULL || regcomp(&regex, pattern, REG_EXTENDED | REG_ICASE) != 0)
		return 0;
	int result = regexec(&regex, url, 0, NULL, 0) == 0;
	regfree(&regex);
	return result;
}

char *request(char *url)
{
	CURL *curl = curl_easy_init();
	if (curl == NULL)
		return NULL;

	downloaded.length = 0;
	if (downloaded.data != NULL)
		downloaded.data[0] = '\0';

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "ABOB/0.0");
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &downloaded);
	curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

	CURLcode result = curl_easy_perform(curl);
	long response_code = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
	curl_easy_cleanup(curl);

	if (result != CURLE_OK || response_code >= 400 || downloaded.data == NULL) {
		fprintf(stderr, "Unable to open %s: %s\n", url,
			result == CURLE_OK ? "HTTP error" : curl_easy_strerror(result));
		return NULL;
	}
	return downloaded.data;
}
