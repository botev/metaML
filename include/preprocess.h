//
// Created by alex on 17/05/16.
//

#ifndef METAML_PREPROCESS_H
#define METAML_PREPROCESS_H

namespace metaMl{
    namespace preprocess {

        af::array get_mean(std::vector <InMemoryDataSource> &sources,
                           std::string variable,
                           std::vector<unsigned> axis,
                           bool mean_over_all = false){
            af::array mean;
            if(mean_over_all){
                af::array partial_mean;
                long long current_count = 0;
                for(auto s = 0; s < sources.size(); s++){
                    long long new_count = 1;
                    partial_mean = sources[s].data[variable].first;
                    for(auto i = 0; i < axis.size(); ++i) {
                        new_count *= mean.dims(axis[i]);
                        partial_mean = af::mean(partial_mean, axis[i]);
                    }
                    mean *= ((float)(current_count)) / ((float)(current_count + new_count));
                    mean += partial_mean * ((float)new_count) / ((float)(current_count + new_count));
                    current_count += new_count;
                }
            } else {
                mean = sources[0].data[variable].first;
                for(auto i = 0; i < axis.size(); ++i) {
                    mean = af::mean(mean, axis[i]);
                }
            }
            return mean;
        }

        af::array get_variance(std::vector <InMemoryDataSource> &sources,
                               std::string variable,
                               std::vector<unsigned> axis,
                               bool mean_over_all = false) {
            af::array mean = get_mean(sources, variable, axis, mean_over_all);
            af::array var;
            if(mean_over_all){
                af::array partial_var;
                long long current_count = 0;
                for(auto s = 0; s < sources.size(); s++){
                    long long new_count = 1;
                    partial_var = sources[s].data[variable].first - mean;
                    partial_var *= partial_var;
                    for(auto i = 0; i < axis.size(); ++i) {
                        new_count *= mean.dims(axis[i]);
                        partial_var = af::mean(partial_var, axis[i]);
                    }
                    var *= ((float)(current_count)) / ((float)(current_count + new_count));
                    var += partial_var * ((float)new_count) / ((float)(current_count + new_count));
                    current_count += new_count;
                }
            } else {
                var = sources[0].data[variable].first - mean;
                var * var;
                for(auto i = 0; i < axis.size(); ++i) {
                    var = af::mean(var, axis[i]);
                }
            }
            return var;
        }

        void center(std::vector <InMemoryDataSource> &sources,
                    std::string variable,
                    std::vector<unsigned> axis,
                    bool mean_over_all = false) {
            af::array mean = get_mean(sources, variable, axis, mean_over_all);
            for (auto i = 0; i < sources.size(); ++i) {
                sources[i].data[variable].first -= mean;
            }
        }

        void normalize(std::vector <InMemoryDataSource> &sources,
                       std::string variable,
                       std::vector<unsigned> axis,
                       bool mean_over_all = false,
                       float epsilon = 1e-9) {
            af::array std = af::sqrt(get_variance(sources, variable, axis, mean_over_all) + epsilon);
            for (auto i = 0; i < sources.size(); ++i) {
                sources[i].data[variable].first /= std;
            }
        }

    }
}
#endif //METAML_PREPROCESS_H
