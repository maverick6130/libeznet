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
    bool dtype() { return type; }
    const header_map& get_headers() { return hds; }
};

class mime_string_part : public mime_part
{
    std::string f;
    std::string d;
public:
    mime_string_part(std::string field, std::string data, header_map headers = header_map()) : mime_part(MIME_STRING, headers), f(field), d(data) {}
    const std::string& get_field() const { return f; }
    const std::string& get_data() const { return d; }
};

class mime_file_part : public mime_part
{
    std::string f;
    std::string pth;
    std::string rmt;
public:
    mime_file_part (std::string field, std::string path, std::string remotename, header_map headers = header_map()) : mime_part(MIME_FILE, headers), f(field), pth(path), rmt(remotename) {}
    const std::string& get_field() const { return f; }
    const std::string& get_path() const { return pth; }
    const std::string& get_remote() const { return rmt; }
};

class http_client
{
private:
    std::string err;
    bool log_en = false;

    static size_t write(void *buffer, size_t size, size_t nmemb, std::string* userp)
    {
        userp->append((char*) buffer, size*nmemb);
        return size*nmemb;
    }
    static size_t writef(void *buffer, size_t size, size_t nmemb, std::ofstream* userp)
    {
        userp->write((char*) buffer, size*nmemb);
        return size*nmemb;
    }
    static size_t read(void *buffer, size_t size, size_t nmemb, std::stringstream* userp)
    {
        userp->read((char*) buffer, size*nmemb);
        return userp->gcount();
    }
    static size_t readf(void *buffer, size_t size, size_t nmemb, FILE* userp)
    {
        int bytes_read = fread(buffer, size, nmemb, userp);
        return bytes_read;
    }
    curl_slist* bna_hds(CURL* hdl, header_map headers) // build and attach headers
    {
        curl_slist* hds = NULL;
        if (!headers.empty())
        {
            for(auto h : headers)
            {
                std::string header = h.first + ":" + h.second;
                hds = curl_slist_append(hds, header.c_str());
            }
            curl_easy_setopt(hdl, CURLOPT_HTTPHEADER, hds);
        }
        return hds;
    }
public:

    int get(std::string url,std::string* response, header_map headers);
    int getfile(std::string url,std::string filename, header_map headers);
    int put(std::string url, std::string data, std::string* response, header_map headers);
    int putfile(std::string url, std::string filename, std::string* response, header_map headers);
    int simplepost(std::string url, std::string data, std::string * response, header_map headers);
    int binarypost(std::string url, void* data, long int size, std::string* response, header_map headers);
    int formpost(std::string url, std::vector<mime_part*> parts, std::string *response, header_map headers);

    int c_get(std::string type, std::string url,std::string* response, header_map headers);
    int c_getfile(std::string type, std::string url,std::string filename, header_map headers);
    int c_put(std::string type, std::string url, std::string data, std::string* response, header_map headers);
    int c_putfile(std::string type, std::string url, std::string filename, std::string* response, header_map headers);
    int c_simplepost(std::string type, std::string url, std::string data, std::string * response, header_map headers);
    int c_binarypost(std::string type, std::string url, void* data, long int size, std::string* response, header_map headers);
    int c_formpost(std::string type, std::string url, std::vector<mime_part*> parts, std::string *response, header_map headers);

    void enable_logging() { log_en = true; }
    void disable_logging() {log_en = false; }
    const char* log_status() { return log_en ? "enabled" : "disabled"; }
    std::string log() { return err; }
    void free_log() { err.erase(); }
};

int http_client::get(std::string url, std::string* response = nullptr, header_map headers = header_map())
{
    if(log_en) err += "get() :\n";
    // handle initialization
    CURL* hdl = curl_easy_init();
    if (!hdl)
    {
        if(log_en) err += "Error in handle initialization\n\n";
        return CURL_BAD_HANDLE;
    }

    // option setting
    curl_easy_setopt(hdl, CURLOPT_URL, url.c_str());
    if (response)
    {
        curl_easy_setopt(hdl, CURLOPT_WRITEFUNCTION, write);
        curl_easy_setopt(hdl, CURLOPT_WRITEDATA, response);
    }
    curl_slist* hds = bna_hds(hdl, headers);

    // perform
    CURLcode res = curl_easy_perform(hdl);
    if(log_en) err += curl_easy_strerror(res);
    if(log_en) err += "\n\n";

    // cleanup
    curl_slist_free_all(hds);
    curl_easy_cleanup(hdl);

    return (int)res;
}

int http_client::c_get(std::string type, std::string url, std::string* response = nullptr, header_map headers = header_map())
{
    if(log_en) err += "get() :\n";
    // handle initialization
    CURL* hdl = curl_easy_init();
    if (!hdl)
    {
        if(log_en) err += "Error in handle initialization\n\n";
        return CURL_BAD_HANDLE;
    }

    // option setting
    curl_easy_setopt(hdl, CURLOPT_URL, url.c_str());
    if (response)
    {
        curl_easy_setopt(hdl, CURLOPT_WRITEFUNCTION, write);
        curl_easy_setopt(hdl, CURLOPT_WRITEDATA, response);
    }
    curl_easy_setopt(hdl, CURLOPT_CUSTOMREQUEST, type.c_str());
    curl_slist* hds = bna_hds(hdl, headers);

    // perform
    CURLcode res = curl_easy_perform(hdl);
    if(log_en) err += curl_easy_strerror(res);
    if(log_en) err += "\n\n";

    // cleanup
    curl_slist_free_all(hds);
    curl_easy_cleanup(hdl);

    return (int)res;
}

int http_client::getfile(std::string url, std::string filename, header_map headers = header_map())
{
    if(log_en) err += "getfile() :\n";
    // handle initialization
    CURL* hdl = curl_easy_init();
    if (!hdl)
    {
        if(log_en) err += "Error in handle initialization\n\n";
        return CURL_BAD_HANDLE;
    }

    // opening file
    std::ofstream file;
    file.open(filename);
    if ( !file )
    {
        if(log_en) err += "Error in opening file\n\n";
        return CURL_FILE_ERR;
    }

    // option setting
    curl_easy_setopt(hdl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(hdl, CURLOPT_WRITEFUNCTION, writef);
    curl_easy_setopt(hdl, CURLOPT_WRITEDATA, &file);
    curl_slist* hds = bna_hds(hdl, headers);

    // perform
    CURLcode res = curl_easy_perform(hdl);
    if(log_en) err += curl_easy_strerror(res);
    if(log_en) err += "\n\n";

    // cleanup
    curl_slist_free_all(hds);
    curl_easy_cleanup(hdl);
    file.close();

    return (int)res;
}

int http_client::c_getfile(std::string type, std::string url, std::string filename, header_map headers = header_map())
{
    if(log_en) err += "getfile() :\n";
    // handle initialization
    CURL* hdl = curl_easy_init();
    if (!hdl)
    {
        if(log_en) err += "Error in handle initialization\n\n";
        return CURL_BAD_HANDLE;
    }

    // opening file
    std::ofstream file;
    file.open(filename);
    if ( !file )
    {
        if(log_en) err += "Error in opening file\n\n";
        return CURL_FILE_ERR;
    }

    // option setting
    curl_easy_setopt(hdl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(hdl, CURLOPT_WRITEFUNCTION, writef);
    curl_easy_setopt(hdl, CURLOPT_WRITEDATA, &file);
    curl_easy_setopt(hdl, CURLOPT_CUSTOMREQUEST, type.c_str());
    curl_slist* hds = bna_hds(hdl, headers);

    // perform
    CURLcode res = curl_easy_perform(hdl);
    if(log_en) err += curl_easy_strerror(res);
    if(log_en) err += "\n\n";

    // cleanup
    curl_slist_free_all(hds);
    curl_easy_cleanup(hdl);
    file.close();

    return (int)res;
}

int http_client::put(std::string url, std::string data, std::string* response = nullptr, header_map headers = header_map())
{
    if(log_en) err += "put()\n";
    // handle initialization
    CURL* hdl = curl_easy_init();
    if (!hdl)
    {
        if(log_en) err += "Error in handle initialization\n\n";
        return CURL_BAD_HANDLE;
    }

    // stringstream for readfunction
    std::stringstream ss(data);


    // option setting
    curl_easy_setopt(hdl, CURLOPT_URL, url.c_str());
    if (response)
    {
        curl_easy_setopt(hdl, CURLOPT_WRITEFUNCTION, write);
        curl_easy_setopt(hdl, CURLOPT_WRITEDATA, response);
    }
    curl_easy_setopt(hdl, CURLOPT_READFUNCTION, read);
    curl_easy_setopt(hdl, CURLOPT_READDATA, &ss);
    curl_easy_setopt(hdl, CURLOPT_INFILESIZE_LARGE, data.size());
    curl_easy_setopt(hdl, CURLOPT_UPLOAD, 1L);
    curl_slist* hds = bna_hds(hdl, headers);

    // perform
    CURLcode res = curl_easy_perform(hdl);
    if(log_en) err += curl_easy_strerror(res);
    if(log_en) err += "\n\n";

    // cleanup
    curl_slist_free_all(hds);
    curl_easy_cleanup(hdl);

    return (int)res;
}

int http_client::c_put(std::string type, std::string url, std::string data, std::string* response = nullptr, header_map headers = header_map())
{
    if(log_en) err += "put()\n";
    // handle initialization
    CURL* hdl = curl_easy_init();
    if (!hdl)
    {
        if(log_en) err += "Error in handle initialization\n\n";
        return CURL_BAD_HANDLE;
    }

    // stringstream for readfunction
    std::stringstream ss(data);


    // option setting
    curl_easy_setopt(hdl, CURLOPT_URL, url.c_str());
    if (response)
    {
        curl_easy_setopt(hdl, CURLOPT_WRITEFUNCTION, write);
        curl_easy_setopt(hdl, CURLOPT_WRITEDATA, response);
    }
    curl_easy_setopt(hdl, CURLOPT_READFUNCTION, read);
    curl_easy_setopt(hdl, CURLOPT_READDATA, &ss);
    curl_easy_setopt(hdl, CURLOPT_INFILESIZE_LARGE, data.size());
    curl_easy_setopt(hdl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(hdl, CURLOPT_CUSTOMREQUEST, type.c_str());
    curl_slist* hds = bna_hds(hdl, headers);

    // perform
    CURLcode res = curl_easy_perform(hdl);
    if(log_en) err += curl_easy_strerror(res);
    if(log_en) err += "\n\n";

    // cleanup
    curl_slist_free_all(hds);
    curl_easy_cleanup(hdl);

    return (int)res;
}

int http_client::putfile(std::string url, std::string filename, std::string* response = nullptr, header_map headers = header_map())
{
    if(log_en) err += "putfile()\n";
    // handle initialization
    CURL* hdl = curl_easy_init();
    if (!hdl)
    {
        if(log_en) err += "Error in handle initialization\n\n";
        return CURL_BAD_HANDLE;
    }

    // file handling for file to PUT
    FILE* file = fopen(filename.c_str(), "rb");
    struct stat fileinfo;
    stat(filename.c_str(), &fileinfo);
    curl_off_t filesz = fileinfo.st_size;

    // option setting
    curl_easy_setopt(hdl, CURLOPT_URL, url.c_str());
    if (response)
    {    
        curl_easy_setopt(hdl, CURLOPT_WRITEFUNCTION, write);
        curl_easy_setopt(hdl, CURLOPT_WRITEDATA, response);
    }
    curl_easy_setopt(hdl, CURLOPT_READFUNCTION, readf);
    curl_easy_setopt(hdl, CURLOPT_READDATA, file);
    curl_easy_setopt(hdl, CURLOPT_INFILESIZE_LARGE, filesz);
    curl_easy_setopt(hdl, CURLOPT_UPLOAD, 1L);
    curl_slist* hds = bna_hds(hdl, headers);

    // perform
    CURLcode res = curl_easy_perform(hdl);
    if(log_en) err += curl_easy_strerror(res);
    if(log_en) err += "\n\n";

    // cleanup
    curl_slist_free_all(hds);
    curl_easy_cleanup(hdl);
    fclose(file);

    return (int)res;
}

int http_client::c_putfile(std::string type, std::string url, std::string filename, std::string* response = nullptr, header_map headers = header_map())
{
    if(log_en) err += "putfile()\n";
    // handle initialization
    CURL* hdl = curl_easy_init();
    if (!hdl)
    {
        if(log_en) err += "Error in handle initialization\n\n";
        return CURL_BAD_HANDLE;
    }

    // file handling for file to PUT
    FILE* file = fopen(filename.c_str(), "rb");
    struct stat fileinfo;
    stat(filename.c_str(), &fileinfo);
    curl_off_t filesz = fileinfo.st_size;

    // option setting
    curl_easy_setopt(hdl, CURLOPT_URL, url.c_str());
    if (response)
    {    
        curl_easy_setopt(hdl, CURLOPT_WRITEFUNCTION, write);
        curl_easy_setopt(hdl, CURLOPT_WRITEDATA, response);
    }
    curl_easy_setopt(hdl, CURLOPT_READFUNCTION, readf);
    curl_easy_setopt(hdl, CURLOPT_READDATA, file);
    curl_easy_setopt(hdl, CURLOPT_INFILESIZE_LARGE, filesz);
    curl_easy_setopt(hdl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(hdl, CURLOPT_CUSTOMREQUEST, type.c_str());
    curl_slist* hds = bna_hds(hdl, headers);

    // perform
    CURLcode res = curl_easy_perform(hdl);
    if(log_en) err += curl_easy_strerror(res);
    if(log_en) err += "\n\n";

    // cleanup
    curl_slist_free_all(hds);
    curl_easy_cleanup(hdl);
    fclose(file);

    return (int)res;
}

int http_client::simplepost(std::string url, std::string data, std::string * response = nullptr, header_map headers = header_map())
{
    if(log_en) err += "simplepost()\n";
    // handle initialization
    CURL* hdl = curl_easy_init();
    if (!hdl)
    {
        if(log_en) err += "Error in handle initialization\n\n";
        return CURL_BAD_HANDLE;
    }

    // option setting
    curl_easy_setopt(hdl, CURLOPT_URL, url.c_str());
    if (response)
    {    
        curl_easy_setopt(hdl, CURLOPT_WRITEFUNCTION, write);
        curl_easy_setopt(hdl, CURLOPT_WRITEDATA, response);
    }
    curl_easy_setopt(hdl, CURLOPT_POSTFIELDS, data.c_str());

    curl_slist* hds = bna_hds(hdl, headers);

    // perform
    CURLcode res = curl_easy_perform(hdl);
    if(log_en) err += curl_easy_strerror(res);
    if(log_en) err += "\n\n";

    // cleanup
    curl_slist_free_all(hds);
    curl_easy_cleanup(hdl);

    return (int)res;
}

int http_client::c_simplepost(std::string type, std::string url, std::string data, std::string * response = nullptr, header_map headers = header_map())
{
    if(log_en) err += "simplepost()\n";
    // handle initialization
    CURL* hdl = curl_easy_init();
    if (!hdl)
    {
        if(log_en) err += "Error in handle initialization\n\n";
        return CURL_BAD_HANDLE;
    }

    // option setting
    curl_easy_setopt(hdl, CURLOPT_URL, url.c_str());
    if (response)
    {    
        curl_easy_setopt(hdl, CURLOPT_WRITEFUNCTION, write);
        curl_easy_setopt(hdl, CURLOPT_WRITEDATA, response);
    }
    curl_easy_setopt(hdl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(hdl, CURLOPT_CUSTOMREQUEST, type.c_str());

    curl_slist* hds = bna_hds(hdl, headers);

    // perform
    CURLcode res = curl_easy_perform(hdl);
    if(log_en) err += curl_easy_strerror(res);
    if(log_en) err += "\n\n";

    // cleanup
    curl_slist_free_all(hds);
    curl_easy_cleanup(hdl);

    return (int)res;
}

int http_client::binarypost(std::string url, void* data, long int size, std::string* response = nullptr, header_map headers = header_map())
{
    if(log_en) err += "binarypost()\n";
    // handle initialization
    CURL* hdl = curl_easy_init();
    if (!hdl)
    {
        if(log_en) err += "Error in handle initialization\n\n";
        return CURL_BAD_HANDLE;
    }

    // option setting
    curl_easy_setopt(hdl, CURLOPT_URL, url.c_str());
    if (response)
    {    
        curl_easy_setopt(hdl, CURLOPT_WRITEFUNCTION, write);
        curl_easy_setopt(hdl, CURLOPT_WRITEDATA, response);
    }
    curl_easy_setopt(hdl, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(hdl, CURLOPT_POSTFIELDSIZE, size);

    curl_slist* hds = bna_hds(hdl, headers);

    // perform
    CURLcode res = curl_easy_perform(hdl);
    if(log_en) err += curl_easy_strerror(res);
    if(log_en) err += "\n\n";

    // cleanup
    curl_slist_free_all(hds);
    curl_easy_cleanup(hdl);

    return (int)res;    
}

int http_client::c_binarypost(std::string type, std::string url, void* data, long int size, std::string* response = nullptr, header_map headers = header_map())
{
    if(log_en) err += "binarypost()\n";
    // handle initialization
    CURL* hdl = curl_easy_init();
    if (!hdl)
    {
        if(log_en) err += "Error in handle initialization\n\n";
        return CURL_BAD_HANDLE;
    }

    // option setting
    curl_easy_setopt(hdl, CURLOPT_URL, url.c_str());
    if (response)
    {    
        curl_easy_setopt(hdl, CURLOPT_WRITEFUNCTION, write);
        curl_easy_setopt(hdl, CURLOPT_WRITEDATA, response);
    }
    curl_easy_setopt(hdl, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(hdl, CURLOPT_POSTFIELDSIZE, size);
    curl_easy_setopt(hdl, CURLOPT_CUSTOMREQUEST, type.c_str());

    curl_slist* hds = bna_hds(hdl, headers);

    // perform
    CURLcode res = curl_easy_perform(hdl);
    if(log_en) err += curl_easy_strerror(res);
    if(log_en) err += "\n\n";

    // cleanup
    curl_slist_free_all(hds);
    curl_easy_cleanup(hdl);

    return (int)res;    
}

int http_client::formpost(std::string url, std::vector<mime_part*> parts, std::string *response = nullptr, header_map headers = header_map())
{
    if(log_en) err += "formpost()\n";
    // handle initialization
    CURL* hdl = curl_easy_init();
    if (!hdl)
    {
        if(log_en) err += "Error in handle initialization\n\n";
        return CURL_BAD_HANDLE;
    }

    // option setting
    curl_easy_setopt(hdl, CURLOPT_URL, url.c_str());
    if (response)
    {    
        curl_easy_setopt(hdl, CURLOPT_WRITEFUNCTION, write);
        curl_easy_setopt(hdl, CURLOPT_WRITEDATA, response);
    }

    curl_mime* mpf = curl_mime_init(hdl);
    for (auto p : parts)
    {   
        curl_mimepart* part = curl_mime_addpart(mpf);
        
        curl_slist* d_hds = NULL;
        if (!p->get_headers().empty())
        {
            for(auto h : p->get_headers())
            {
                std::string header = h.first + ":" + h.second;
                d_hds = curl_slist_append(d_hds, header.c_str());
            }
            curl_mime_headers(part, d_hds, true);
        }

        if ( p->dtype() == MIME_STRING )
        {
            mime_string_part* cp = (mime_string_part*) p;
            curl_mime_name(part, cp->get_field().c_str());
            curl_mime_data(part, cp->get_data().c_str(), CURL_ZERO_TERMINATED);
        }
        else
        {
            mime_file_part* cp = (mime_file_part*) p;
            curl_mime_name(part, cp->get_field().c_str());
            curl_mime_filename(part, cp->get_remote().c_str());
            curl_mime_filedata(part, cp->get_path().c_str());
        }
    }

    curl_easy_setopt(hdl, CURLOPT_MIMEPOST, mpf);

    curl_slist* hds = bna_hds(hdl, headers);

    // perform
    CURLcode res = curl_easy_perform(hdl);
    if(log_en) err += curl_easy_strerror(res);
    if(log_en) err += "\n\n";

    // cleanup
    curl_slist_free_all(hds);
    curl_easy_cleanup(hdl);
    curl_mime_free(mpf);

    return (int)res;    
}

int http_client::c_formpost(std::string type, std::string url, std::vector<mime_part*> parts, std::string *response = nullptr, header_map headers = header_map())
{
    if(log_en) err += "formpost()\n";
    // handle initialization
    CURL* hdl = curl_easy_init();
    if (!hdl)
    {
        if(log_en) err += "Error in handle initialization\n\n";
        return CURL_BAD_HANDLE;
    }

    // option setting
    curl_easy_setopt(hdl, CURLOPT_URL, url.c_str());
    if (response)
    {    
        curl_easy_setopt(hdl, CURLOPT_WRITEFUNCTION, write);
        curl_easy_setopt(hdl, CURLOPT_WRITEDATA, response);
    }

    curl_mime* mpf = curl_mime_init(hdl);
    for (auto p : parts)
    {   
        curl_mimepart* part = curl_mime_addpart(mpf);
        
        curl_slist* d_hds = NULL;
        if (!p->get_headers().empty())
        {
            for(auto h : p->get_headers())
            {
                std::string header = h.first + ":" + h.second;
                d_hds = curl_slist_append(d_hds, header.c_str());
            }
            curl_mime_headers(part, d_hds, true);
        }

        if ( p->dtype() == MIME_STRING )
        {
            mime_string_part* cp = (mime_string_part*) p;
            curl_mime_name(part, cp->get_field().c_str());
            curl_mime_data(part, cp->get_data().c_str(), CURL_ZERO_TERMINATED);
        }
        else
        {
            mime_file_part* cp = (mime_file_part*) p;
            curl_mime_name(part, cp->get_field().c_str());
            curl_mime_filename(part, cp->get_remote().c_str());
            curl_mime_filedata(part, cp->get_path().c_str());
        }
    }

    curl_easy_setopt(hdl, CURLOPT_MIMEPOST, mpf);
    curl_easy_setopt(hdl, CURLOPT_CUSTOMREQUEST, type.c_str());

    curl_slist* hds = bna_hds(hdl, headers);

    // perform
    CURLcode res = curl_easy_perform(hdl);
    if(log_en) err += curl_easy_strerror(res);
    if(log_en) err += "\n\n";

    // cleanup
    curl_slist_free_all(hds);
    curl_easy_cleanup(hdl);
    curl_mime_free(mpf);

    return (int)res;    
}
