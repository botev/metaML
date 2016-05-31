//
// Created by alex on 23/05/16.
//

#ifndef METAML_MNIST_H
#define METAML_MNIST_H


namespace metaMl {
    namespace datasets {
        namespace os = metadiff::os;
        class MNISTDataset : public AbstractStandardDataset{
        private:
            const std::string train_images_url = "http://yann.lecun.com/exdb/mnist/train-images-idx3-ubyte.gz";
            const std::string train_labels_url = "http://yann.lecun.com/exdb/mnist/train-labels-idx1-ubyte.gz";
            const std::string test_images_url = "http://yann.lecun.com/exdb/mnist/t10k-images-idx3-ubyte.gz";
            const std::string test_labels_url = "http://yann.lecun.com/exdb/mnist/t10k-labels-idx1-ubyte.gz";
            const std::string archive_train_images_file = "train_images.gz";
            const std::string archive_train_labels_file = "train_labels.gz";
            const std::string archive_test_images_file = "test_images.gz";
            const std::string archive_test_labels_file = "test_labels.gz";
            const std::string train_images_file = "train_images";
            const std::string train_labels_file = "train_labels";
            const std::string test_images_file = "test_images";
            const std::string test_labels_file = "test_labels";
            const off_t train_images_checksum = 47040016;
            const off_t train_labels_checksum  = 60008;
            const off_t test_images_checksum = 7840016;
            const off_t test_labels_checksum  = 10008;
            const int validation_sizes[10] = {987, 1125, 993, 1022, 973, 903, 986, 1045, 975, 991};

            int reverse_int(int i){
                unsigned char ch1, ch2, ch3, ch4;
                ch1=i&255;
                ch2=(i>>8)&255;
                ch3=(i>>16)&255;
                ch4=(i>>24)&255;
                return((int)ch1<<24)+((int)ch2<<16)+((int)ch3<<8)+ch4;
            }

            af::array load_images(std::string file_name){
                std::ifstream file(file_name,std::ios::binary);
                if (file.is_open())
                {
                    int magic_number=0;
                    int number_of_images=0;
                    int n_rows=0;
                    int n_cols=0;
                    file.read((char*)&magic_number,sizeof(magic_number));
                    magic_number= reverse_int(magic_number);
                    file.read((char*)&number_of_images,sizeof(number_of_images));
                    number_of_images= reverse_int(number_of_images);
                    file.read((char*)&n_rows,sizeof(n_rows));
                    n_rows= reverse_int(n_rows);
                    file.read((char*)&n_cols,sizeof(n_cols));
                    n_cols= reverse_int(n_cols);
                    float * data = new float[n_rows*n_cols*number_of_images];
                    unsigned char temp=0;
                    for(auto i=0;i<number_of_images;++i) {
                        for(auto r=0;r<n_rows;++r) {
                            for(auto c=0;c<n_cols;++c) {
                                temp = 0;
                                file.read((char*)&temp,sizeof(temp));
                                data[(n_rows*c)+r + i*n_rows*n_cols]= ((float)temp) / float(255.0);
                            }
                        }
                    }
                    file.close();
                    af::array array = af::array(n_rows, n_cols, number_of_images, data, afHost);
                    delete[] data;
                    return array;
                }
                return af::constant(0, 0);
            }

            af::array load_labels(std::string file_name){
                std::ifstream file(file_name,std::ios::binary);
                if (file.is_open())
                {
                    int magic_number=0;
                    int number_of_images=0;
                    file.read((char*)&magic_number,sizeof(magic_number));
                    magic_number= reverse_int(magic_number);
                    file.read((char*)&number_of_images,sizeof(number_of_images));
                    number_of_images= reverse_int(number_of_images);
                    unsigned short * data = new unsigned short[number_of_images];
                    for(auto i=0;i<number_of_images;++i) {
                        unsigned char temp=0;
                        file.read((char*)&temp,sizeof(temp));
                        data[i]= (unsigned short)temp;
                    }
                    af::array array = af::array(number_of_images, data, afHost);
                    delete[] data;
                    return array;
                }
                return af::constant(0, 0);
            }
        public:
            MNISTDataset() : AbstractStandardDataset("MNIST") {};

            bool verify_downloaded(std::string path){
                std::string temp = os::join_paths(path, train_images_file);
                if(not os::exists(temp) or os::file_size(temp) != train_images_checksum){
                    return false;
                }
                temp = os::join_paths(path, train_labels_file);
                if(not os::exists(temp) or os::file_size(temp) != train_labels_checksum){
                    return false;
                }
                temp = os::join_paths(path, test_images_file);
                if(not os::exists(temp) or os::file_size(temp) != test_images_checksum){
                    return false;
                }
                temp = os::join_paths(path, test_labels_file);
                if(not os::exists(temp) or os::file_size(temp) != test_labels_checksum){
                    return false;
                }
                return true;
            }

            void download(std::string path){
                logger()->debug() << "Downloading data to " << path;
                os::create_dir(path, true);
                
                // Train images
                logger()->debug() << "Downloading train images...";
                std::string temp = os::join_paths(path, archive_train_images_file);
                os::download_file(train_images_url, temp);
                os::unpack_gz(temp);
                // Train labels
                logger()->debug() << "Downloading train labels...";
                temp = os::join_paths(path, archive_train_labels_file);
                os::download_file(train_labels_url, temp);
                os::unpack_gz(temp);
                // Test images
                logger()->debug() << "Downloading test images...";
                temp = os::join_paths(path, archive_test_images_file);
                os::download_file(test_images_url, temp);
                os::unpack_gz(temp);
                // Test labels
                logger()->debug() << "Downloading test labels...";
                temp = os::join_paths(path, archive_test_labels_file);
                os::download_file(test_labels_url, temp);
                os::unpack_gz(temp);
            }

            void load(std::string path){
                {
                    logger()->debug() << "Loading test images...";
                    af::array test_images = load_images(os::join_paths(path, test_images_file));
                    logger()->debug() << "Loading test labels...";
                    af::array test_labels = load_labels(os::join_paths(path, test_labels_file));
                    test = InMemoryDataSource();
                    test.add_data("images", test_images, 2);
                    test.add_data("labels", test_labels, 0);
                }
                logger()->debug() << "Loading training images...";
                af::array train_images = load_images(os::join_paths(path, train_images_file));
                logger()->debug() << "Loading training labels...";
                af::array train_labels = load_labels(os::join_paths(path, train_labels_file));
                af::array train_idx = train_labels > -1;
                {
                    // Split training set to train and validation
                    af::array valid_idx = af::where(train_labels == 0)(af::seq(validation_sizes[0]));
                    for (auto i = 1; i < 10; ++i) {
                        valid_idx = af::join(0, valid_idx, af::where(train_labels == i)(af::seq(validation_sizes[i])));
                    }
                    // Sort the validation index
                    valid_idx = af::sort(valid_idx);
                    af::array valid_images = train_images(af::span, af::span, valid_idx, af::span);
                    af::array valid_labels = train_labels(valid_idx);
                    valid = InMemoryDataSource();
                    valid.add_data("images", valid_images, 2);
                    valid.add_data("labels", valid_labels, 0);
                    // Remove validation images from the training data
                    train_idx(valid_idx) = 0;
                }
                train_images = train_images(af::span, af::span, train_idx, af::span);
                train_labels = train_labels(train_idx);
                train = InMemoryDataSource();
                train.add_data("images", train_images, 2);
                train.add_data("labels", train_labels, 0);
            }
        };

        static Dataset MNIST(){
            return std::make_shared<MNISTDataset>();
        }
    }
}
#endif //METAML_MNIST_H
