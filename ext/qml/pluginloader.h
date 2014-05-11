#pragma once

#include "rubyclassbase.h"
#include <QtCore/QPluginLoader>
#include <memory>

namespace RubyQml {

class PluginLoader : public RubyClassBase<PluginLoader>
{
    friend class RubyClassBase<PluginLoader>;

public:
    PluginLoader();
    ~PluginLoader();

    VALUE initialize(VALUE path);
    VALUE load();
    VALUE instance();

private:
    void mark() {}
    static ClassBuilder buildClass();
    std::unique_ptr<QPluginLoader> mPluginLoader;
};

} // namespace RubyQml
