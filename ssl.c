#include <stdio.h>
#include <curl/curl.h>
#include <string.h>

int main(void)
{
    CURL *curl;
    CURLcode res;
    struct curl_httppost *formpost=NULL;
    struct curl_httppost *lastptr=NULL;
    struct curl_slist *headerlist=NULL;
      static const char buf[] = "Expect:";


    curl = curl_easy_init();
    if(curl)
    {  headerlist = curl_slist_append(headerlist, buf);

        curl_easy_setopt(curl, CURLOPT_URL, "https://openvz.brodul.org/");
        curl_easy_setopt(curl, CURLOPT_PORT, 443);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        /*curl_easy_setopt(curl, CURLOPT_CAINFO, );*/
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L );

        curl_formadd(&formpost,
            &lastptr,
            CURLFORM_COPYNAME, "rfid",
            CURLFORM_ARRAY, "blabla",
            CURLFORM_END);

        curl_formadd(&formpost,
            &lastptr,
            CURLFORM_COPYNAME, "MAC",
            CURLFORM_ARRAY, "blabla",
            CURLFORM_END);

        res = curl_easy_perform(curl);
        printf("%d", res);
        curl_easy_cleanup(curl);
    }
}
