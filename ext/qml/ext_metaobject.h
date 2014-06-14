#pragma once

#include "extbase.h"
#include <QtCore/QMetaObject>
#include <QtCore/QVector>
#include <QtCore/QHash>
#include <QtCore/QObject>

namespace RubyQml {
namespace Ext {

class MetaObject : public ExtBase<MetaObject>
{
    friend class ExtBase<MetaObject>;
public:

    MetaObject();

    RubyValue className() const;

    RubyValue methodNames() const;
    RubyValue isPublic(RubyValue name) const;
    RubyValue isProtected(RubyValue name) const;
    RubyValue isPrivate(RubyValue name) const;
    RubyValue isSignal(RubyValue name) const;

    RubyValue invokeMethod(RubyValue object, RubyValue methodName, RubyValue args) const;
    RubyValue connectSignal(RubyValue object, RubyValue signalName, RubyValue proc) const;

    RubyValue propertyNames() const;
    RubyValue getProperty(RubyValue object, RubyValue name) const;
    RubyValue setProperty(RubyValue object, RubyValue name, RubyValue newValue) const;
    RubyValue notifySignal(RubyValue name) const;

    RubyValue enumerators() const;

    RubyValue superClass() const;

    RubyValue isEqual(RubyValue other) const;
    RubyValue hash() const;

    void setMetaObject(const QMetaObject *metaObject);
    const QMetaObject *metaObject() const { return mMetaObject; }

    RubyValue buildRubyClass();

    static RubyValue fromMetaObject(const QMetaObject *metaObject);
    static void initClass();

private:

    void mark() {}

    QList<int> findMethods(RubyValue name) const;
    int findProperty(RubyValue name) const;

    const QMetaObject *mMetaObject = nullptr;
    QMultiHash<ID, int> mMethodHash;
    QHash<ID, int> mPropertyHash;
};

} // namespace Ext
} // namespace RubyQml
