#ifndef __HTTP_CLIENT_HPP__
#define __HTTP_CLIENT_HPP__

#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <curl/curl.h>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef header_map
#define header_map std::map<std::string, std::string>
#endif

#define CURL_BAD_HANDLE -1
#define CURL_FILE_ERR -2

#define MIME_STRING 0
#define MIME_FILE 1

class mime_part
{
    header_map hds;
    bool type;

public:
    mime_part(int t, header_map headers) : type(t), hds(headers) {}
    inline bool dtype() { return type; }
    inline const header_map &get_headers() { return hds; }
};

class mime_string_part : public mime_part
{
    std::string f;
    std::string d;

public:
    mime_string_part (
        std::string field, 
        std::string data, 
        header_map headers = header_map()
    ) : mime_part(MIME_STRING, headers), f(field), d(data) {}
    inline const std::string &get_field() const { return f; }
    inline const std::string &get_data() const { return d; }
};

class mime_file_part : public mime_part
{
    std::string f;
    std::string pth;
    std::string rmt;

public:
    mime_file_part ( 
        std::string field, 
        std::string path, 
        std::string remotename, 
        header_map headers = header_map()
    ) : mime_part(MIME_FILE, headers), f(field), pth(path), rmt(remotename) {}
    inline const std::string &get_field() const { return f; }
    inline const std::string &get_path() const { return pth; }
    inline const std::string &get_remote() const { return rmt; }
};

class http_client
{

private:
    std::string err;
    bool log_en = false;

    inline static size_t write(void *buffer, size_t size, size_t nmemb, std::string *userp)
    {
        userp->append((char *)buffer, size * nmemb);
        return size * nmemb;
    }
    inline static size_t writef(void *buffer, size_t size, size_t nmemb, std::ofstream *userp)
    {
        userp->write((char *)buffer, size * nmemb);
        return size * nmemb;
    }
    inline static size_t read(void *buffer, size_t size, size_t nmemb, std::stringstream *userp)
    {
        userp->read((char *)buffer, size * nmemb);
        return userp->gcount();
    }
    inline static size_t readf(void *buffer, size_t size, size_t nmemb, FILE *userp)
    {
        int bytes_read = fread(buffer, size, nmemb, userp);
        return bytes_read;
    }
    inline static curl_slist* bna_hds(CURL *hdl, header_map headers) // build and attach headers
    {
        curl_slist *hds = NULL;
        if (!headers.empty())
        {
            for (auto h : headers)
            {
                std::string header = h.first + ":" + h.second;
                hds = curl_slist_append(hds, header.c_str());
            }
            curl_easy_setopt(hdl, CURLOPT_HTTPHEADER, hds);
        }
        return hds;
    }

public:
    int get (
        const std::string& url, 
        long &rescode, 
        std::string *response = nullptr, 
        header_map headers = header_map(),
        const std::string& custom_method = "GET"
    );
    int getfile ( 
        const std::string& url, 
        long &rescode, 
        const std::string& filename, 
        header_map headers = header_map(),
        const std::string& custom_method = "GET"
    );
    int put ( 
        const std::string& url, 
        long &rescode, 
        const std::string& data, 
        std::string *response = nullptr, 
        header_map headers = header_map(),
        const std::string& custom_method = "PUT"
    );
    int putfile ( 
        const std::string& url, 
        long &rescode, 
        const std::string& filename, 
        std::string *response = nullptr, 
        header_map headers = header_map(),
        const std::string& custom_method = "PUT"
    );
    int simplepost ( 
        const std::string& url, 
        long &rescode, 
        const std::string& data, 
        std::string *response = nullptr, 
        header_map headers = header_map(),
        const std::string& custom_method = "POST"
    );
    int binarypost ( 
        const std::string& url, 
        long &rescode, 
        void *data, 
        long int size, 
        std::string *response = nullptr, 
        header_map headers = header_map(),
        const std::string& custom_method = "POST"
    );
    int formpost ( 
        const std::string& url, 
        long &rescode, 
        const std::vector<mime_part *>& parts, 
        std::string *response = nullptr, 
        header_map headers = header_map(),
        const std::string& custom_method = "POST"
    );

    inline void enable_logging() { log_en = true; }
    inline void disable_logging() { log_en = false; }
    inline const char *log_status() { return log_en ? "enabled" : "disabled"; }
    inline std::string log() { return err; }
    inline void free_log() { err.clear(); }
};

#endif