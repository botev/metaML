//
// Created by alex on 16/05/16.
//

#ifndef METAML_SOURCE_H
#define METAML_SOURCE_H

namespace metaMl{
    class InMemoryDataSource{
    public:
        long n;
        std::map<std::string, std::pair<af::array, unsigned>> data;
        std::vector<std::string> current_iterates;

        InMemoryDataSource(long n = 0): n(0) {};

        void add_data(std::string name, af::array value, unsigned int instance_dim){
            if(n == 0){
                n = value.dims(instance_dim);
            }
            if(value.dims(instance_dim) != n){
                throw 2;
            }
            if(data.find(name) != data.end()){
                throw 1;
            }
            data[name] = {value, instance_dim};
        }

        void shuffle_data(af::array order) {
            for (auto it = data.begin(); it != data.end(); ++it) {
                unsigned dim = it->second.second;
                switch (dim) {
                    case 0: {
                        it->second = {it->second.first(order, af::span, af::span, af::span), dim};
                        break;
                    }
                    case 1: {
                        it->second = {it->second.first(af::span, order, af::span, af::span), dim};
                        break;
                    }
                    case 2: {
                        it->second = {it->second.first(af::span, af::span, order, af::span), dim};
                        break;
                    }
                    case 3: {
                        it->second = {it->second.first(af::span, af::span, af::span, order), dim};
                        break;
                    }
                    default: {
                        throw 2;
                    }
                }
            }
        };
//            if(instance_dim == 0){
////                    auto f = [&order](std::pair<std::string const, af::array>& value){
////                        value.second = value.second(order, af::span, af::span, af::span);
////                    };
//                for(auto it = data.begin(); it != data.end(); ++it){
//                    it->second = it->second(order, af::span, af::span, af::span);
//                }
////                    std::for_each(data.begin(), data.end(), f);
//            } else if(instance_dim == 1){
////                    auto f = [&order](std::pair<std::string const, af::array>& value){
////                        value.second = value.second(af::span, order, af::span, af::span);
////                    };
//                for(auto it = data.begin(); it != data.end(); ++it){
//                    it->second = it->second(af::span, order, af::span, af::span);
//                }
//            } else if(instance_dim == 2){
////                    auto f = [&order](std::pair<std::string const, af::array>& value){
////                        value.second = value.second(af::span, af::span, order, af::span);
////                    };
//                for(auto it = data.begin(); it != data.end(); ++it){
//                    it->second = it->second(af::span, af::span, order, af::span);
//                }
//            } else if(instance_dim == 3){
//                std::cout << "Shuffle 3" << std::endl;
////                    auto f = [&order](std::pair<std::string const, af::array>& value){
////                        value.second = value.second(af::span, af::span, af::span, order);
////                    };
//                for(auto it = data.begin(); it != data.end(); ++it){
//                    it->second = it->second(af::span, af::span, af::span, order);
//                }
//            };
//        }

        void random_shuffle(){
            af::array rand =  af::randu(n);
            af::array order;
            af::sort(rand, order, rand, 0);
            shuffle_data(order);
        }

        class iterator {
        protected:
            InMemoryDataSource &source;
            unsigned batch_size;
            bool full_batches;

            int index;
            std::vector<af::array> slice;
            bool updated;
        public:
            typedef std::vector<af::array> value_type;
            typedef std::vector<af::array> &ref_type;
            typedef std::vector<af::array> *ptr_type;

            iterator(InMemoryDataSource &source, int index,
                     unsigned batch_size,
                     bool full_batches) :
                    source(source), index(index),
                    batch_size(batch_size),
                    full_batches(full_batches), updated(false) {
                if(batch_size == 0){
                    this->batch_size = source.n;
                }
            };

            iterator(iterator const &ref) :
                    iterator(ref.source, ref.index, ref.batch_size, ref.full_batches){};

            iterator& operator=(iterator const & ref){
                this->source = ref.source;
                this->batch_size = ref.batch_size;
                this->full_batches = ref.full_batches;
                this->index = ref.index;
                this->slice = ref.slice;
                this->updated = ref.updated;
                return *this;
            }
            virtual bool operator==(iterator const &ref) {
                return &source == &ref.source and index == ref.index;
            }

            virtual bool operator!=(iterator const &ref) {
                return &source != &ref.source or index != ref.index;
            }

            iterator& operator++() {
                index+=batch_size;
                if(index > source.n or (full_batches and index + batch_size >= source.n)){
                    index = source.n;
                }
                updated = false;
                return *this;
            }

            iterator operator++(int) {
                iterator copy(*this);
                ++(*this);
                return copy;
            }

            virtual ref_type operator*() {
                fetch_slice();
                return slice;
            }

            virtual ptr_type operator->() {
                fetch_slice();
                return &slice;
            }

            virtual void fetch_slice(){
                if(not updated) {
                    slice.clear();
                    int last = (index+batch_size-1) < source.n ? (index+batch_size-1) : source.n-1;
                    for (auto i = 0; i < source.current_iterates.size(); ++i) {
                        unsigned dim = source.data[source.current_iterates[i]].second;
                        switch (dim){
                            case 0 :{
                                slice.push_back(source.data[source.current_iterates[i]].first(af::seq(index, last), af::span, af::span, af::span));
                                break;
                            } case 1: {
                                slice.push_back(source.data[source.current_iterates[i]].first(af::span, af::seq(index, last), af::span, af::span));
                                break;
                            } case 2: {
                                slice.push_back(source.data[source.current_iterates[i]].first(af::span, af::span, af::seq(index, last), af::span));
                                break;
                            } case 3: {
                                slice.push_back(source.data[source.current_iterates[i]].first(af::span, af::span, af::span, af::seq(index, last)));
                                break;
                            }
                            default: {
                                throw 2;
                            }
                        }
                    }
                    updated = true;
                }
            }
        };

        iterator begin( int batch_size, bool full_batches = false){
            return iterator(*this, 0, batch_size, full_batches);
        }

        iterator end(){
            return iterator(*this, n, 0, false);
        }

        void print(){
            for(auto i = data.begin(); i != data.end(); ++i){
                std::cout << i->first << std::endl;
                af_print(i->second.first);
            }
        }
    };

    std::ostream &operator<<(std::ostream &f,  const InMemoryDataSource & source) {
        auto it=source.data.begin();
        f << " " << it->first << ":" << "(" << it->second.first.dims() << ")";
        ++it;
        for(; it != source.data.end(); ++it){
            f << " || " << it->first << ":" << "(" << it->second.first.dims() << ")";
        }
        return f;
    }
}

#endif //METAML_SOURCE_H
