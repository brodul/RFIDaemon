#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

int main(void)
{
  CURL *curl;
  CURLcode res;
  struct curl_slist *headers=NULL;
  headers = curl_slist_append(headers, "Content-Type: application/json");
  char json[128];
  curl = curl_easy_init();
  if(curl) {
    /* First set the URL that is about to receive our POST. This URL can
       just as well be a https:// URL if that is what should receive the
       data. */

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L );
    /*curl_easy_setopt(curl, CURLOPT_URL, "https://openvz.brodul.org");*/
    curl_easy_setopt(curl, CURLOPT_URL, "localhost/listener.php");
    /*curl_easy_setopt(curl, CURLOPT_URL, "localhost");*/
    /*curl_easy_setopt(curl, CURLOPT_URL, "http://www.postbin.org/qpss4f");*/
    curl_easy_setopt(curl, CURLOPT_PORT, 8000 );
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    strcpy (json,"{ ");
    strcat (json,"\"mac\" : ");
    strcat (json,"\"00:22:69:67:e8:dd\"");
    strcat (json,", \"rfid\" : ");
    strcat (json,"\"6100800\"");
    strcat (json,"}");



    /* Now specify the POST data */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json );

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    printf("%d", res);

    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  return 0;
}

