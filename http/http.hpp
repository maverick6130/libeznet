#ifndef __HTTP_HPP__
#define __HTTP_HPP__

#include <curl/curl.h>
#include <vector>
#include <unordered_map>
#include <string>
#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define header_map std::unordered_map<std::string, std::string>

// ** form part helper class and specialization for multi part formposts **//

#define MIME_STRING 0
#define MIME_FILE 1

class formpart
{
    header_map hds;
    bool type;

public:
    formpart(int t, header_map headers) : type(t), hds(headers) {}
    inline bool dtype() { return type; }
    inline const header_map &get_headers() { return hds; }
};

class formpart_s : public formpart
{
    std::string f;
    std::string d;

public:
    formpart_s (
        std::string field, 
        std::string data, 
        header_map headers = header_map()
    ) : formpart(MIME_STRING, headers), f(field), d(data) {}
    inline const std::string &get_field() const { return f; }
    inline const std::string &get_data() const { return d; }
};

class formpart_f : public formpart
{
    std::string f;
    std::string pth;
    std::string rmt;

public:
    formpart_f ( 
        std::string field, 
        std::string path, 
        std::string remotename, 
        header_map headers = header_map()
    ) : formpart(MIME_FILE, headers), f(field), pth(path), rmt(remotename) {}
    inline const std::string &get_field() const { return f; }
    inline const std::string &get_path() const { return pth; }
    inline const std::string &get_remote() const { return rmt; }
};

// ** http request class and specializations **//

class http_request
{
    std::string method;
    header_map headers;
    std::string target;
};

// ** http response class and specializations ** //

class http_response
{
protected:
    bool noError;
    std::string error;
    long code;
    header_map headers;
    http_response (bool noErr, const std::string& err, long rescode, const header_map& hds)
    : noError(noErr), error(err), code(rescode), headers(hds) {}
public:
    inline bool no_error() {return noError;}
    inline std::string get_error() { return error; }
    inline long get_code() { return code; }
    inline const std::string& get_header(std::string val)
    {
        if (headers.find(val) != headers.end())
            return headers[val];
        return "";
    }
    inline const header_map& get_headers() { return headers; }
};

class http_string_response : public http_response
{
    std::string body;
public:
    inline const std::string& get_body() { return body; }
};

class http_file_response : public http_response
{
    std::string filename;
public:
    inline const std::string& get_filename() { return filename; }
};

#endif