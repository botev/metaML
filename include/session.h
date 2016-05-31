//
// Created by alex on 16/05/16.
//

#ifndef METAML_SESSION_H
#define METAML_SESSION_H

namespace metaMl{
    typedef metadiff::api::AfBackend Func;
    class FunctionExecution{
    public:
        bool initial;
        bool final;
        unsigned period;
        unsigned batch_size;
        Func function;
        InMemoryDataSource source;

        void eval_full(){
            for(auto it = source.begin(batch_size); it != source.end(); ++it){
                function.eval(*it);
            }
        }
    };

    class Session{
    public:
        unsigned snapshot_period;
        std::vector<metadiff::api::SharedPtr> parameters;
        FunctionExecution main_function;
        std::vector<FunctionExecution> extra_functions;
        Session() {};

        virtual void run(unsigned steps, bool epochs_based = false) = 0;
        virtual void build() = 0;
    };

    class StandardSession: public Session{
    public:
        StandardSession() {};

        void build(){

        }

        void run(unsigned steps, bool epochs_based = false){
            // Pre-execute any of the extra extra_functions before the run
            for(auto j = 0; j < extra_functions.size(); ++j){
                if(extra_functions[j].initial){
                    // TODO somehow to store the results
                    extra_functions[j].eval_full();
                }
            }
            // Main loop
            auto it = main_function.source.begin(main_function.batch_size);
            for(auto i = 1; i <= steps; ++i){
                // Run any of the extra function before the main
                for(auto j = 0; j < extra_functions.size(); ++j){
                    if(extra_functions[j].period > 0 and i % extra_functions[j].period == 0){
                        // Execute function
                    }
                }

                if(epochs_based){
                    for(;it != main_function.source.end(); ++it){
                        // TODO somehow to store the results
                        main_function.function.eval(*it);
                    }
                } else {
                    // TODO somehow to store the results
                    main_function.function.eval(*it);
                }
                if(it == main_function.source.end()){
                    it = main_function.source.begin(main_function.batch_size);
                }

                // TODO check for any validations such ass NaNs/Infs

                if(i % snapshot_period == 0){
                    // TODO Take a snapshot of any of the parameters
                }
            }
            // Post-execute any of the extra extra_functions after the run
            for(auto j = 0; j < extra_functions.size(); ++j){
                if(extra_functions[j].final){
                    // TODO somehow to store the results
                    extra_functions[j].eval_full();
                }
            }
        }
    };
}


#endif //METAML_SESSION_H
