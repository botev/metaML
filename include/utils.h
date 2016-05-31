//
// Created by alex on 23/05/16.
//

#ifndef METAML_UTILS_H
#define METAML_UTILS_H
#include "curl/curl.h"
#include <sys/stat.h>
#include "fstream"

namespace metaMl{
    namespace utils{
        const char kPathSeparator =
#ifdef _WIN32
                '\\';
#else
                '/';
#endif
        /** The name of the environmental variable expected for a http proxy */
        const std::string HTTP_PROXY = "HTTP_PROXY";

        /** The name of the environmental variable expected for the path to the datasets folder */
        const std::string DATASETS_PATH = "DATASETS_PATH2";

        /** Helper for writing to files */
        struct FtpFile {
            const char *filename;
            FILE *stream;
        };

        /** Helper writes stream to files */
        static size_t write_steram_to_file(void *buffer, size_t size, size_t nmemb, void *stream) {
            struct FtpFile *out=(struct FtpFile *)stream;
            if(out && !out->stream) {
                out->stream=fopen(out->filename, "wb");
                if(!out->stream)
                    return -1; /* failure, can't open file to write */
            }
            return fwrite(buffer, size, nmemb, out->stream);
        }

        /** Checks if a the file specified by the path exists on the file system */
        bool exists_file(std::string path){
            struct stat buffer;
            return (stat (path.c_str(), &buffer) == 0);
        }

        /** Checks if a the directory specified by the path exists on the file system */
        bool exists_dir(std::string path){
            struct stat info;
            if(stat( path.c_str(), &info ) != 0)
                return 0;
            else if(info.st_mode & S_IFDIR)
                return 1;
            else
                return 0;
        }

        /** Helper function to create directories */
        void create_dir(std::string path) {
            const int dir_err = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            if (-1 == dir_err) {
                std::cout << "Error creating directory " << path << std::endl;
            }
        }

        long long file_size(std::string file_name) {
            struct stat st;
            if (stat(file_name.c_str(), &st) == -1)
                return 0;
            return ((long long)st.st_size);
        }

        /** Joins os paths **/
        std::string join_paths(std::string path1, std::string path2){
            if(path1.back() == kPathSeparator){
                if(path2.front() == kPathSeparator) {
                    return path1.substr(0, path1.size() - 1) + path2;
                } else {
                    return path1 + path2;
                }
            } else if (path2.front() == kPathSeparator){
                return path1 + path2;
            } else {
                return path1 + kPathSeparator + path2;
            }
        }

        /** Joins os paths **/
        std::string join_paths(std::vector<std::string> paths){
            if(paths.size() == 0){
                return "";
            } else if(paths.size() == 1){
                return paths[0];
            } else{
                std::string path = paths[0];
                for(auto i = 0; i < paths.size(); ++i){
                    path = join_paths(path, paths[i]);
                }
                return path;
            }
        }

        /** A helper function to download a file from a url */
        void download_file(std::string url, std::string local_path){
            CURL *curl;
            CURLcode res;
            curl_global_init(CURL_GLOBAL_DEFAULT);
            curl = curl_easy_init();
            struct FtpFile data={local_path.c_str(), NULL};
            if(curl) {
                if(std::getenv(HTTP_PROXY.c_str())){
                    curl_easy_setopt(curl, CURLOPT_PROXY, std::getenv(HTTP_PROXY.c_str()));
                }
                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write_steram_to_file);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
                res = curl_easy_perform(curl);
                if(CURLE_OK != res) {
                    fprintf(stderr, "curl told us %d\n", res);
                }
            }
            /* always cleanup */
            curl_easy_cleanup(curl);
            if(data.stream) {
                fclose(data.stream);
            }
            curl_global_cleanup();
        }

        /** A helper function to unpack gz files */
        int unpack_gz(std::string gz_path){
            return system(("gzip -d -f " + gz_path).c_str());
        }

    }
}
#endif //METAML_UTILS_H
