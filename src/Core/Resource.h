#include <stdio.h>
#include <string>

class Resource {
public:
    Resource();
    ~Resource();

    template <class S>
    S* load(const std::string& _path, const std::string& _name) {
        T t = new T();
        return t.load(_path);
    }

};