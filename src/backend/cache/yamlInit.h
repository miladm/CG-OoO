#ifndef __YAML_INIT_H__
#define __YAML_INIT_H__

#include <yaml/yaml.h>
#include <map>
#include <string>

namespace Memory {

class YAMLInitable {
    virtual void init(const YAML::Node &) =0;
};

template<class Base>
class GeneratorBase {
    public:
        virtual Base *generate(const YAML::Node &root) =0;
};

template<class Base>
class Factory {
    public:
        typedef typename std::map<std::string, GeneratorBase<Base>*> GenMap;
    private:
        static GenMap *gens_;
        Factory() {}
    public:
        static void add_generator(GeneratorBase<Base> *gen, const std::string &name) {
            if(gens_ == NULL)
                gens_ = new std::map<std::string, GeneratorBase<Base>*>();
            (*gens_)[name] = gen;
        }

        static Base *generate(const std::string &name, const YAML::Node &root) {
            return (*gens_)[name]->generate(root);
        }
};

template<class Base>
typename Factory<Base>::GenMap *Factory<Base>::gens_ = NULL;

template<class Base, class YAMLInitableClass>
class Generator : public GeneratorBase<Base> {
    public:
        Generator(const std::string &name) {
            Factory<Base>::add_generator(this, name);
        }

        virtual Base *generate(const YAML::Node &root) {
            auto *ptr = new YAMLInitableClass();
            ptr->init(root);
            return ptr;
        }
};

}

#endif
