#include "http_client.hpp"

int http_client::get(const std::string& url, long &rescode, std::string* response, header_map headers, const std::string& type)
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
    curl_easy_getinfo(hdl, CURLINFO_RESPONSE_CODE, &rescode);
    if(log_en) err += curl_easy_strerror(res);
    if(log_en) err += "\n\n";

    // cleanup
    curl_slist_free_all(hds);
    curl_easy_cleanup(hdl);

    return (int)res;
}

int http_client::getfile (const std::string& url, long &rescode, const std::string& filename, header_map headers, const std::string& type)
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
    curl_easy_getinfo(hdl, CURLINFO_RESPONSE_CODE, &rescode);
    if(log_en) err += curl_easy_strerror(res);
    if(log_en) err += "\n\n";

    // cleanup
    curl_slist_free_all(hds);
    curl_easy_cleanup(hdl);
    file.close();

    return (int)res;
}

int http_client::put(const std::string& url, long &rescode, const std::string& data, std::string* response, header_map headers, const std::string& type)
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
    curl_easy_getinfo(hdl, CURLINFO_RESPONSE_CODE, &rescode);
    if(log_en) err += curl_easy_strerror(res);
    if(log_en) err += "\n\n";

    // cleanup
    curl_slist_free_all(hds);
    curl_easy_cleanup(hdl);

    return (int)res;
}

int http_client::putfile(const std::string& url, long &rescode, const std::string& filename, std::string* response, header_map headers, const std::string& type)
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
    curl_easy_getinfo(hdl, CURLINFO_RESPONSE_CODE, &rescode);
    if(log_en) err += curl_easy_strerror(res);
    if(log_en) err += "\n\n";

    // cleanup
    curl_slist_free_all(hds);
    curl_easy_cleanup(hdl);
    fclose(file);

    return (int)res;
}

int http_client::simplepost(const std::string& url, long &rescode, const std::string& data, std::string * response, header_map headers, const std::string& type)
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
    curl_easy_getinfo(hdl, CURLINFO_RESPONSE_CODE, &rescode);
    if(log_en) err += curl_easy_strerror(res);
    if(log_en) err += "\n\n";

    // cleanup
    curl_slist_free_all(hds);
    curl_easy_cleanup(hdl);

    return (int)res;
}

int http_client::binarypost(const std::string& url, long &rescode, void* data, long int size, std::string* response, header_map headers, const std::string& type)
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
    curl_easy_getinfo(hdl, CURLINFO_RESPONSE_CODE, &rescode);
    if(log_en) err += curl_easy_strerror(res);
    if(log_en) err += "\n\n";

    // cleanup
    curl_slist_free_all(hds);
    curl_easy_cleanup(hdl);

    return (int)res;    
}

int http_client::formpost(const std::string& url, long &rescode, const std::vector<mime_part*>& parts, std::string *response, header_map headers, const std::string& type)
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
    curl_easy_getinfo(hdl, CURLINFO_RESPONSE_CODE, &rescode);
    if(log_en) err += curl_easy_strerror(res);
    if(log_en) err += "\n\n";

    // cleanup
    curl_slist_free_all(hds);
    curl_easy_cleanup(hdl);
    curl_mime_free(mpf);

    return (int)res;    
}