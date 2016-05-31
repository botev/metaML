//
// Created by alex on 17/05/16.
//
#include <unistd.h>
#include "metaML.h"

namespace ml = metaMl;

int main()
{
    af::array s = af::seq(1, 19885);
    af::array s1 = af::seq(1, 19885) * 0.2;
    af::array s2 = af::seq(1, 19885) / 19885 * (0.2);
    std::cout << s.dims() << " - " << s1.dims() << " - " << s2.dims() << std::endl;
    spdlog::set_pattern("*** [%H:%M:%S %z] [thread %t] %v ***");
    metadiff::logging::metadiff_sink->add_sink(std::make_shared<spdlog::sinks::stdout_sink_st>());
    std::string name = "PATH";
    auto source = ml::InMemoryDataSource();
    source.add_data("d1", af::randu(5, 7), 1);
    source.add_data("d2", af::randu(3, 7), 1);
    source.add_data("d3", af::randu(1, 7), 1);
    source.print();
    source.random_shuffle();
    source.print();
    source.current_iterates = {"d2", "d3"};
    for(auto i = source.begin(2, true); i != source.end(); ++i){
        std::cout << "Iteration "  << std::endl;
        for(auto j = 0; j < i->size(); ++j){
            af_print((*i)[j]);
        }
    }
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
        fprintf(stdout, "Current working dir: %s\n", cwd);
    else
        perror("getcwd() error");
    std::string nme(cwd);
    nme = "./datasets.o";
    ml::datasets::Dataset mnist = ml::datasets::MNIST();
    mnist->init();
    af::Window wnd(1120, 1120, "Preview");
    wnd.grid(4, 10);
    mnist->train.current_iterates = {"images", "labels"};
//    mnist->train.random_shuffle();
    for(auto i = mnist->train.begin(40); i != mnist->train.end(); ++i){
        if(wnd.close()){
            break;
        }
        for(auto j = 0; j < 40 ; ++j){
            std::string title = "Class: " + std::to_string((*i)[1](j).host<unsigned short>()[0]);
            wnd(j / 10, j % 10).image((*i)[0].slice(j), title.c_str());
        }
        wnd.show();
        usleep(2000000);
    }
}
