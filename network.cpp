#include "network.h"

#include <boost/property_tree/json_parser.hpp>
#include <curl/curl.h>

std::ostream&
operator<<(std::ostream& os, const PTree& root)
{
    boost::property_tree::write_json(os, root);
    return os;
}

HTTPConnection::HTTPConnection(const std::string& server) :
    server(server),
    curl(curl_easy_init())
{
    assert(curl);
}

HTTPConnection::~HTTPConnection() {
    curl_easy_cleanup(curl);
}

// Helper functions for libcurl
struct MemoryStruct {
    MemoryStruct() : memory((char*)malloc(1)), size(0) {}
    ~MemoryStruct() {if(memory) { free(memory); }}
    char *memory;
    size_t size;
};
static size_t
WriteMemoryCallback(char *contents, size_t size, size_t nmemb, void *userp)
{
    const int MAX_SIZE = 1e6;
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    if (mem->size + realsize + 1 > MAX_SIZE) {
        std::cerr << "Excessive response size > " << MAX_SIZE << std::endl;
        return 0;
    }
    mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
    if(mem->memory == NULL) {
        /* out of memory! */ 
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}

std::string
HTTPConnection::get_http(const std::string& end_point, const Params& params)
{
    std::stringstream result;
    std::stringstream params_stream;
    for (Params::const_iterator ip = params.begin(), ipe=params.end(); ip!=ipe;)
    {
        params_stream << ip->first << "=" << ip->second;
        ip ++;
        if (ip == ipe) break;
        params_stream << "&";
    }
    std::string post_string = params_stream.str();
    std::stringstream url;
    url << "http://" << server << end_point;
    // std::cout << "POST values: " << post_string.c_str() << std::endl;
    // std::cout << "URL: " << url.str() << std::endl;

    if (!this->proxy.empty()) {
        curl_easy_setopt(curl,CURLOPT_PROXY,this->proxy.c_str());
        curl_easy_setopt(curl,CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4); 
    }
    curl_easy_setopt(curl,CURLOPT_URL,url.str().c_str());
    // curl_easy_setopt(curl, CURLOPT_VERBOSE,1L); // Make curl output debug information
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION,1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS,post_string.c_str());

    //Prepare buffer for the response
    MemoryStruct chunk;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,(void*)&chunk);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "Error performing POST to " << url.str() << std::endl;
        exit(1);
    }
    long http_error_code = -1;
    curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE,&http_error_code);

    // std::cout << "Got the response, processing it..." << std::endl;
    // std::cout.flush();

    std::string response(chunk.memory,chunk.size);

    if (http_error_code != 200) {
        std::cout << "HTTP Error code: " << http_error_code << std::endl;
        std::cout << "Response body: " << response << std::endl;
        exit(1);
    }

    return response;
}

boost::property_tree::ptree
HTTPConnection::get_json(const std::string& end_point, const Params& params)
{
    std::string json_string = get_http(end_point, params);
    std::stringstream json_stream;
    json_stream << json_string;
    boost::property_tree::ptree root;
    boost::property_tree::read_json(json_stream, root);

    return root;
}

PTree
HTTPConnection::get_initial_state_json(const Options& options)
{
    Params params;
    params["key"] = options.secret_key;
    std::string end_point = "/api/arena";

    if (options.training_mode)
    {
        if (options.number_of_turns > 0) params["turns"] = to_string(options.number_of_turns);
        if (!options.map_name.empty()) params["map"] = options.map_name;
        end_point = "/api/training";
    }

    std::cout << "getting initial game state at " << options.server_name << end_point << std::endl;
    std::cout << "using the following parameters: " << std::endl;
    for (Params::const_iterator pi=params.begin(), pie=params.end(); pi!=pie; pi++)
        std::cout << "  " << pi->first << " " << pi->second << std::endl;

    return get_json(end_point, params);
}

PTree
HTTPConnection::get_new_state_json(const std::string& end_point, const Direction& direction)
{
    Params params;
    params["dir"] = to_string(direction);

    //std::cout << "playing " << direction << " at " << server_name << end_point << std::endl;

    return get_json(end_point, params);
}

Position
get_position(const PTree& root)
{
    return Position(root.get<int>("x"), root.get<int>("y"));
}

Tiles
get_tiles(const PTree& root)
{
    return parse_tiles(root.get<int>("size"), root.get<std::string>("tiles"));
}

Tiles
get_background_tiles(const PTree& root)
{
    return neutralize_tiles(get_tiles(root));
}

OwnedMines
get_owned_mines(const PTree& root)
{
    return extract_owned_mines(get_tiles(root));
}
