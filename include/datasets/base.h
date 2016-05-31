//
// Created by alex on 17/05/16.
//

#ifndef METAML_BASE_H
#define METAML_BASE_H

namespace metaMl{
    namespace datasets{
        /** The name of the environmental variable expected for the path to the datasets folder */
        const std::string DATASETS_PATH = "DATASETS_PATH2";

        class AbstractStandardDataset{
        public:
            std::shared_ptr<spdlog::logger> logger() const {
                return metadiff::logging::logger("ml::dataset::" + this->name);
            }
            std::string name;
            InMemoryDataSource train;
            InMemoryDataSource valid;
            InMemoryDataSource test;

            AbstractStandardDataset(std::string name): name(name) {};
            virtual bool verify_downloaded(std::string path) = 0;
            bool verify_downloaded(){
                return verify_downloaded(get_path());
            }

            virtual void download(std::string path) = 0;
            void download(){
                download(get_path());
            }

            virtual void load(std::string path) = 0;
            void load(){
                return load(get_path());
            }

            virtual void init(std::string path){
                while(not verify_downloaded(path)){
                    download(path);
                }
                load(path);
                logger()->info() << "Initialization successful:";
                logger()->info() << "Train: " << train;
                logger()->info() << "Valid: " << valid;
                logger()->info() << "Test : " << test;
            }

            void init(){
                return init(get_path());
            }

        protected:
            std::string get_path(){
                if(std::getenv(DATASETS_PATH.c_str())){
                    return metadiff::os::join_paths(std::getenv(DATASETS_PATH.c_str()), name);
                } else {
                    logger()->warn() << "Environmental variable '" << DATASETS_PATH
                    << " is not set. Falling back to the default one ~/datasets";
                    return metadiff::os::join_paths({"~", "datasets", name});
                }
            }
        };

        typedef std::shared_ptr<AbstractStandardDataset> Dataset;
    }
}
#endif //METAML_BASE_H
